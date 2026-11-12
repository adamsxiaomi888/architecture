#!/usr/bin/env python3
"""Re-run a fixed scenario and compare every JSONL observation (same build)."""
import json
import subprocess
import sys
from pathlib import Path

def main():
    if len(sys.argv) != 3:
        raise ValueError('usage: replay.py /path/to/er_sim_runner trace.jsonl')
    recorded = [json.loads(line) for line in Path(sys.argv[2]).read_text().splitlines()]
    if not recorded or recorded[0].get('schema') != 1:
        raise ValueError('unsupported or empty trace')
    scenario = recorded[0].get('scenario')
    if scenario not in ('normal', 'stale', 'sensor', 'stall'):
        raise ValueError('unknown scenario')
    result = subprocess.run([sys.argv[1], scenario], text=True, capture_output=True, check=True)
    actual = [json.loads(line) for line in result.stdout.splitlines()]
    if len(actual) != len(recorded):
        raise ValueError('trace length mismatch')
    for i, (expected, observed) in enumerate(zip(recorded, actual)):
        if expected != observed:
            raise ValueError(f'replay mismatch at record {i}')
    print(f'replay passed: {scenario}, {len(actual)-1} observations')

if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print(f'replay failed: {error}', file=sys.stderr)
        sys.exit(1)
