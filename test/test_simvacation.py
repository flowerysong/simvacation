#!/usr/bin/env python3

import json
import os
import subprocess

from email.parser import Parser as EMailParser

import pytest


def test_config_nonexist(tool_path):
    res = subprocess.run(
        [
            tool_path('simvacation'),
            '-c', '/thisisanonexistentfile.conf',
            '-f', 'testuser@example.com',
            'testuser',
        ]
    )
    assert res.returncode == os.EX_TEMPFAIL


def test_simple(run_simvacation, testmsg, tmp_path_factory):
    tmpdir = str(tmp_path_factory.mktemp('mailout'))
    res = run_simvacation(
        'testsender@example.com',
        'testrcpt',
        str(testmsg),
        tmpdir,
    )
    with open(os.path.join(tmpdir, 'sendmail.args'), 'r') as f:
        args = json.load(f)
    with open(os.path.join(tmpdir, 'sendmail.input'), 'r') as f:
        content = EMailParser().parse(f)

    assert args[2] == ''
    assert args[3] == 'testsender@example.com'

    assert content['from'] == '"testrcpt" <testrcpt@umich.edu>'
    assert content['to'] == 'testsender@example.com'
    assert content['subject'] == 'Out of email contact (Re: simta test message for test_simple)'
    assert content['auto-submitted'] == 'auto-replied'
    assert content.get_payload().splitlines() == [
        'I am currently out of email contact.',
        'Your mail will be read when I return.',
    ]
