import errno
import json
import os
import signal
import socket
import subprocess
import tempfile
import time

import pytest


def pytest_collect_file(parent, path):
    if os.access(str(path), os.X_OK):
        if path.basename.startswith('int_'):
            return ExternalFile(path, parent)
        if path.basename.startswith('cmocka_'):
            return CMockaFile(path, parent)


class CMockaFile(pytest.File):
    def collect(self):
        os.environ['CMOCKA_MESSAGE_OUTPUT'] = 'TAP'
        try:
            out = subprocess.check_output(str(self.fspath))
        except:
            # This is inefficient and will run the test twice, but eh.
            yield ExternalItem(self.fspath.basename, self, str(self.fspath))
        else:
            for line in out.split('\n'):
                if not line.startswith('ok'):
                    continue
                yield CMockaItem(self, line)

class CMockaItem(pytest.Item):
    def __init__(self, parent, line):
        name = line.split(' - ')[1]
        super(CMockaItem, self).__init__(name, parent)
        self.line = line

    def runtest(self):
        pass


class ExternalFile(pytest.File):
    def collect(self):
        yield ExternalItem(self.fspath.basename, self, str(self.fspath))


class ExternalItem(pytest.Item):
    def __init__(self, name, parent, execpath):
        super(ExternalItem, self).__init__(name, parent)
        self.execpath = execpath

    def runtest(self):
        subprocess.check_output(self.execpath)

    def repr_failure(self, excinfo):
        if isinstance(excinfo.value, subprocess.CalledProcessError):
            return excinfo.value.output


def openport(port):
    # Find a usable port by iterating until there's an unconnectable port
    while True:
        try:
            conn = socket.create_connection(('localhost', port), 0.1)
            port += 1
            if port > 65535:
                raise ValueError("exhausted TCP port range without finding a free one")
        except socket.error:
            return( port )


@pytest.fixture
def redis():
    port = openport(6379)

    devnull = open(os.devnull, 'w')
    redis_proc = None
    try:
        redis_proc = subprocess.Popen(['redis-server', '--port', str(port)], stdout=devnull, stderr=devnull)
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise
        pytest.skip('redis-server not found')

    running = False
    i = 0
    while not running:
        i += 1
        try:
            conn = socket.create_connection(('localhost', port), 0.1)
            running = True
        except socket.error:
            if i > 20:
                raise
            time.sleep(0.1)

    yield(port)

    if redis_proc:
        redis_proc.terminate()


@pytest.fixture(scope='session')
def tool_path():
    def _tool_path(tool):
        binpath = os.path.dirname(os.path.realpath(__file__))
        binpath = os.path.join(binpath, '..', tool)
        return os.path.realpath(binpath)
    return _tool_path
