#!/usr/bin/env python3

"""A simple python script template.
"""

import os
import sys
import argparse
from pathlib import Path
from typing import List

import torch
import torchaudio


def get_wav_files(input_dir: str):
    """
    Get all wav files in a directory
    """
    return [str(f) for f in Path(input_dir).glob("**/*.wav")]


def process_files(wav_files: List[str]):
    """
    Process a list of wav files
    """
    for f in wav_files:
        audio, sample_rate = torchaudio.load(f)
        print(sample_rate)


def main(arguments):

    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("input_dir", help="Input directory", type=str)
    parser.add_argument("outfile", help="Output JSON file name", type=str)

    args = parser.parse_args(arguments)

    # Get all wav files in the input directory
    wav_files = get_wav_files(args.input_dir)

    # Process all the files
    process_files(wav_files)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
