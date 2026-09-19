"""Exercise the actual shell orchestration with fake host tools, no apt/root changes."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'install/build-image-payload.sh'

class PayloadTest(unittest.TestCase):
    def run_helper(self, missing=False):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            bindir = root / 'bin'
            bindir.mkdir()
            tools = {
                'uname': 'echo Linux', 'id': 'echo 1000',
                'sudo': 'echo unexpected-sudo >&2; exit 91',
                'pkg-config': 'exit 0',
                'ctest': 'exit 0',
                'ldd': 'echo "libmosquitto.so => /lib/libmosquitto.so"',
                'dpkg-query': 'echo package-versions',
                'sha256sum': 'test -f "$1" && echo digest',
                'cmake': '''case "$1" in
-S)
  test "$2" = "$EXPECTED_SOURCE" || exit 92
  test -f "$2/src/MqttHealthPublisher.cpp" || exit 93
  case "$*" in *-DGRIDEX_REQUIRE_MQTT=ON*) ;; *) exit 94;; esac
  mkdir -p "$4"
  if [ "$MISSING_BINARY" = 0 ]; then
    touch "$4/gridex_rockpie_service"
    chmod +x "$4/gridex_rockpie_service"
  fi;;
--build) test -d "$2";;
--install)
  mkdir -p "$DESTDIR/usr/local/bin"
  cp "$2/gridex_rockpie_service" "$DESTDIR/usr/local/bin/";;
*) exit 95;;
esac''',
            }
            for name, body in tools.items():
                file = bindir / name
                file.write_text('#!/bin/sh\nset -eu\n' + body + '\n')
                file.chmod(0o755)
            env = {**os.environ, 'PATH': str(bindir) + ':' + os.environ['PATH'],
                   'TMPDIR': str(root), 'EXPECTED_SOURCE': str(SCRIPT.parent.parent),
                   'MISSING_BINARY': '1' if missing else '0'}
            return subprocess.run(['sh', str(SCRIPT), '--skip-dependencies'],
                                  cwd=root, env=env, capture_output=True, text=True)

    def test_real_script_selects_rock_project_and_stages(self):
        result = self.run_helper()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('IMAGE_PAYLOAD_READY=', result.stdout)

    def test_no_payload_success_without_rock_binary(self):
        result = self.run_helper(missing=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('ROCK service missing', result.stderr)
        self.assertNotIn('IMAGE_PAYLOAD_READY=', result.stdout)

if __name__ == '__main__':
    unittest.main()
