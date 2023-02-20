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

# Only analyze the first half second
ANALYSIS_SAMPLES = SAMPLE_RATE // 2


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
    features = []
    for f in tqdm(wav_files):
        audio, sample_rate = torchaudio.load(f)
        # Resample if needed
        if sample_rate != SAMPLE_RATE:
            audio = torchaudio.transforms.Resample(sample_rate, SAMPLE_RATE)(audio)

        # Trim the audio
        audio = audio[:, :ANALYSIS_SAMPLES]

        # Get the mel spectrogram
        mel_spec = mel(audio)
        mel_stats = temporal_stats(mel_spec)
        features.append(mel_stats)

    # Stack all the features
    features = torch.vstack(features)

    return features


def temporal_stats(x: torch.Tensor):

    assert x.ndim == 3, "Input tensor must be 3D: (batch, freq_bins, frames)"

    mean = x.mean(dim=2)
    std = x.std(dim=2)
    mean_diff = x.diff(dim=2).mean(dim=2)
    std_diff = x.diff(dim=2).std(dim=2)

    # Stack
    stats = torch.stack([mean, std, mean_diff, std_diff], dim=-1)
    return stats


def normalize_features(features: torch.Tensor):
    """
    Normalize the features
    """
    assert features.ndim == 3, "Input tensor must be 3D: (batch, freq_bins, stats)"

    for i in range(features.shape[-1]):
        features[:, :, i] = (features[:, :, i] - features[:, :, i].min()) / (
            features[:, :, i].max() - features[:, :, i].min()
        )
        assert torch.all(features[:, :, i] >= 0) and torch.all(features[:, :, i] <= 1)

    assert torch.all(features >= 0) and torch.all(features <= 1)
    return features


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
    features = process_files(wav_files)

    # Normalize the features
    features = normalize_features(features)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
