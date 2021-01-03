#!/usr/bin/env python3
#
# This file is part of Rayni.
#
# Copyright (C) 2018-2021 Martin Ejdestig <marejde@gmail.com>
#
# Rayni is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Rayni is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Rayni. If not, see <http://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# pylint: disable=missing-docstring,too-few-public-methods

import gzip
import hashlib
import os
import subprocess
import sys
import tarfile
import urllib.request
import zipfile

from typing import BinaryIO, Iterable, Optional


class DownloadFile:
    def __init__(self,
                 url: str,
                 digest: str,
                 dest_dir_path: str,
                 archive_member: Optional[str] = None) -> None:
        self.url = url
        self.digest = digest
        self.dest_dir_path = dest_dir_path
        self.archive_member = archive_member


_SPONZA_DOWNLOAD_FILES = [
    DownloadFile('http://www.crytek.com/download/sponza_obj.rar',
                 '0add5d571196781fa3e819821d1b313c5dc0670bb8c89c5705d1cf9c9baa451b',
                 'sponza'),

    DownloadFile('http://www.crytek.com/download/sponza_textures.rar',
                 '31d4bbf3748d7099cc0afeb452d7a6959d15455b8e3be167e35e4bffd2c738c4',
                 'sponza')
]

_STANFORD_DOWNLOAD_FILES = [
    DownloadFile('http://graphics.stanford.edu/pub/3Dscanrep/bunny.tar.gz',
                 'a5720bd96d158df403d153381b8411a727a1d73cff2f33dc9b212d6f75455b84',
                 'stanford',
                 'bunny/reconstruction/bun_zipper.ply'),

    DownloadFile('http://graphics.stanford.edu/pub/3Dscanrep/dragon/dragon_recon.tar.gz',
                 '74ac1d90989c9b1732edee82d57e9ce71452144cf4355f108d8c9c616d28d02f',
                 'stanford',
                 'dragon_recon/dragon_vrip.ply'),

    DownloadFile('http://graphics.stanford.edu/pub/3Dscanrep/happy/happy_recon.tar.gz',
                 '409cd294efbfd8244e15a382b95a9423f153b7776e736c9b09f19ec9d3c10ed0',
                 'stanford',
                 'happy_recon/happy_vrip.ply'),

    DownloadFile('http://graphics.stanford.edu/data/3Dscanrep/lucy.tar.gz',
                 'c4beb1f7bfa965643bbbf889bd1849a4b4b955e95c731941be61e6edac65616a',
                 'stanford'),

    DownloadFile('http://graphics.stanford.edu/data/3Dscanrep/xyzrgb/xyzrgb_dragon.ply.gz',
                 '8aa449f1966cbb50e5896ecc32cf57ab5f0cdfd3c3e37d3e6f60b948997da5c1',
                 'stanford'),

    DownloadFile('http://graphics.stanford.edu/data/3Dscanrep/xyzrgb/xyzrgb_statuette.ply.gz',
                 '1d867b6540c02935caa777bd6746429a62d4a5d23f11c9bfdfebbaa90c05ca8b',
                 'stanford')
]

_DOWNLOAD_FILES = _SPONZA_DOWNLOAD_FILES + _STANFORD_DOWNLOAD_FILES


def _print_no_newline(string: str):
    print(string, end='', flush=True)


def _read_chunks(file: BinaryIO) -> Iterable[bytes]:
    while True:
        data = file.read(0xffff)
        if not data:
            break
        yield data


def _hash_file(path: str) -> str:
    _print_no_newline(f'Hashing {path}...')

    sha256 = hashlib.sha256()

    with open(path, 'rb') as file:
        for data in _read_chunks(file):
            sha256.update(data)

    print('done')

    return sha256.hexdigest()


def _download(url: str, dest_path: str):
    _print_no_newline(f'Downloading {url}...')

    os.makedirs(os.path.dirname(dest_path), exist_ok=True)

    with open(dest_path, 'wb') as dest, urllib.request.urlopen(url) as src:
        content_length = src.info().get('Content-Length', 0)
        bytes_read = 0
        for data in _read_chunks(src):
            dest.write(data)
            bytes_read += len(data)
            _print_no_newline(f'\rDownloading {url}: {bytes_read}/{content_length}...')

    print('done')


def _gzip_extract(archive_path: str, dest_dir_path: str):
    os.makedirs(dest_dir_path, exist_ok=True)

    basename = os.path.basename(archive_path)
    stem, _ = os.path.splitext(basename)
    dest_path = os.path.join(dest_dir_path, stem)

    with open(dest_path, 'wb') as dest, gzip.open(archive_path, 'rb') as gzip_file:
        for data in _read_chunks(gzip_file):
            dest.write(data)


def _rar_extract(archive_path: str, dest_dir_path: str, archive_member: Optional[str] = None):
    os.makedirs(dest_dir_path, exist_ok=True)

    args = ['unrar', 'x', '-y', '-idq', '--', archive_path]
    if archive_member:
        args.append(archive_member)
    args.append(dest_dir_path)

    subprocess.run(args, check=True)


def _extract(archive_path: str, dest_dir_path: str, archive_member: Optional[str] = None):
    _print_no_newline(f'Extracting {archive_path} to {dest_dir_path}...')

    if tarfile.is_tarfile(archive_path):
        with tarfile.open(archive_path, 'r') as tar_file:
            if archive_member:
                tar_file.extract(archive_member, dest_dir_path)
            else:
                tar_file.extractall(dest_dir_path)

    elif zipfile.is_zipfile(archive_path):
        with zipfile.ZipFile(archive_path, 'r') as zip_file:
            if archive_member:
                zip_file.extract(archive_member, dest_dir_path)
            else:
                zip_file.extractall(dest_dir_path)

    elif os.path.splitext(archive_path)[1] == '.gz':
        # Note that .tar.gz files are handled by tarfile module above. This is for non
        # tar files that have been compressed with gzip.
        assert not archive_member
        _gzip_extract(archive_path, dest_dir_path)

    elif os.path.splitext(archive_path)[1] == '.rar':
        _rar_extract(archive_path, dest_dir_path, archive_member)

    else:
        raise RuntimeError(f'extraction of {archive_path} not supported')

    print('done')


def _download_and_extract(download_file: DownloadFile,
                          download_dir_path: str,
                          scenes_dir_path: str):
    basename = os.path.basename(download_file.url)
    dest_path = os.path.join(download_dir_path, download_file.dest_dir_path, basename)
    digest = _hash_file(dest_path) if os.path.exists(dest_path) else None

    if digest != download_file.digest:
        _download(download_file.url, dest_path)

        digest = _hash_file(dest_path)
        if digest != download_file.digest:
            raise RuntimeError(f'{download_file.url} hash mismatch ({digest}, '
                               f'expected {download_file.digest})')

    _extract(dest_path,
             os.path.join(scenes_dir_path, download_file.dest_dir_path),
             download_file.archive_member)


def main():
    if len(sys.argv) != 3:
        print('Simple script for downloading scene data available on the\n'
              'Internet that is too large to have in repo. May create a\n'
              'submodule repo or have some other way for storing scene data\n'
              'at some later point but for now, this is not just good, it\'s\n'
              'good enough! (Have backups if files are removed online.)\n')

        print('Usage: {} <download dir> <scenes dir>'.format(sys.argv[0]))

        sys.exit(1)

    download_dir_path = sys.argv[1]
    scenes_dir_path = sys.argv[2]

    for download_file in _DOWNLOAD_FILES:
        _download_and_extract(download_file, download_dir_path, scenes_dir_path)


if __name__ == "__main__":
    main()
