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
from tqdm import tqdm

SAMPLE_RATE = 48000


def get_wav_files(input_dir: str):
    """
    Get all wav files in a directory
    """
    return list(Path(input_dir).glob("**/*.wav"))


def process_files(wav_files: List[str]):
    """
    Process a list of wav files
    """
    mel = torchaudio.transforms.MelSpectrogram(
        sample_rate=SAMPLE_RATE, n_fft=2048, hop_length=128, n_mels=128
    )
    for f in tqdm(wav_files):
        audio, sample_rate = torchaudio.load(f)
        # Resample if needed
        if sample_rate != SAMPLE_RATE:
            audio = torchaudio.transforms.Resample(sample_rate, SAMPLE_RATE)(audio)

        # Get the mel spectrogram
        mel_spec = mel(audio)


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
