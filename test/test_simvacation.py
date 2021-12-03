#!/usr/bin/env python3

import json
import os
import subprocess
import time

from email.parser import Parser as EMailParser


def _run_simvacation(
    run_simvacation,
    testmsg,
    tmp_path_factory,
    sender='testsender@example.com',
    rcpt='testrcpt',
):
    tmpdir = str(tmp_path_factory.mktemp('mailout'))
    run_simvacation(
        sender,
        rcpt,
        str(testmsg),
        tmpdir,
    )

    args = None
    content = None

    try:
        with open(os.path.join(tmpdir, 'sendmail.args'), 'r') as f:
            args = json.load(f)
    except FileNotFoundError:
        pass

    try:
        with open(os.path.join(tmpdir, 'sendmail.input'), 'r') as f:
            content = EMailParser().parse(f)
    except FileNotFoundError:
        pass

    return {
        'args': args,
        'content': content,
    }


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
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)

    assert res['args'][2] == ''
    assert res['args'][3] == 'testsender@example.com'

    assert res['content']['from'] == 'testrcpt@example.com'
    assert res['content']['to'] == 'testsender@example.com'
    assert res['content']['subject'] == 'Out of email contact (Re: simta test message for test_simple)'
    assert res['content']['auto-submitted'] == 'auto-replied'
    assert res['content'].get_payload().splitlines() == [
        'I am currently out of email contact.',
        'Your mail will be read when I return.',
    ]


def test_suppress(run_simvacation, testmsg, tmp_path_factory):
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)

    assert res['args'] is None
    assert res['content'] is None


def test_suppress_interval(run_simvacation, testmsg, tmp_path_factory):
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)

    assert res['args'] is None
    assert res['content'] is None

    time.sleep(2)
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory)

    assert res['args']
    assert res['content']


def test_ldap_simple(run_simvacation, testmsg, tmp_path_factory):
    testmsg['To'] = 'onvacation@example.com'
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory, rcpt='onvacation')

    assert res['content']['from'] == 'onvacation@example.com'
    assert res['content'].get_payload().splitlines() == [
        'I am currently out of email contact.',
        'Your mail will be read when I return.',
    ]


def test_ldap_not_to(run_simvacation, testmsg, tmp_path_factory):
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory, rcpt='onvacation')

    assert res['args'] is None
    assert res['content'] is None


def test_ldap_custom(run_simvacation, testmsg, tmp_path_factory):
    testmsg['To'] = 'customvacation@example.com'
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory, rcpt='customvacation')

    assert res['content']['from'] == '"Testy User" <customvacation@example.com>'
    assert res['content'].get_payload().splitlines() == [
        'I am out of the office for till college.',
        'Please contact the uncaring universe (-dev.null@umich.edu) for assistance.',
    ]


def test_ldap_not_on_vacation(run_simvacation, testmsg, tmp_path_factory):
    testmsg['To'] = 'flowerysong@example.com'
    res = _run_simvacation(run_simvacation, testmsg, tmp_path_factory, rcpt='flowerysong')

    assert res['args'] is None
    assert res['content'] is None
