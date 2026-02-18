#!/usr/bin/env python3
"""
ffmpeg-unit-analyzer.py
Analyzes FFmpeg audio filter JSON to extract and categorize parameter unit types.

Usage: python3 ffmpeg-unit-analyzer.py [input.json]
       Default input: ffmpeg-audio-filters.json

Outputs analysis to stdout and optionally to a JSON/MD file.
"""

import json
import re
import sys
from collections import defaultdict


def analyze_units(data):
    """Analyze all parameters and extract unit information."""
    
    # Track findings
    ffmpeg_types = defaultdict(list)       # ffmpeg_type -> [(filter, param)]
    detected_units = defaultdict(list)      # unit -> [(filter, param, context)]
    range_patterns = defaultdict(list)      # pattern -> [(filter, param)]
    default_patterns = defaultdict(list)    # pattern -> [(filter, param)]
    
    # Unit detection patterns in descriptions
    unit_patterns = [
        (r'\b(\d+)\s*dB\b', 'dB'),
        (r'\bdB\b', 'dB'),
        (r'\b(\d+)\s*Hz\b', 'Hz'),
        (r'\bHz\b', 'Hz'),
        (r'\bfrequency\b', 'Hz (implied)'),
        (r'\bcutoff\b', 'Hz (implied)'),
        (r'\bbandwidth\b', 'Hz (implied)'),
        (r'\b(\d+)\s*ms\b', 'ms'),
        (r'\bmilliseconds?\b', 'ms'),
        (r'\bseconds?\b', 'seconds'),
        (r'\bsamples?\b', 'samples'),
        (r'\b(\d+)\s*%\b', 'percent'),
        (r'\bpercent\b', 'percent'),
        (r'\bratio\b', 'ratio'),
        (r'\bgain\b', 'gain'),
        (r'\blevel\b', 'level'),
        (r'\bvolume\b', 'volume'),
        (r'\bthreshold\b', 'threshold'),
        (r'\battack\b', 'time (attack)'),
        (r'\brelease\b', 'time (release)'),
        (r'\bdelay\b', 'time (delay)'),
        (r'\bduration\b', 'time (duration)'),
        (r'\bspeed\b', 'speed/rate'),
        (r'\brate\b', 'rate'),
        (r'\bdepth\b', 'depth'),
        (r'\bmix\b', 'mix/blend'),
        (r'\bwet\b', 'wet/dry'),
        (r'\bdry\b', 'wet/dry'),
        (r'\bwidth\b', 'width'),
        (r'\boctave\b', 'octaves'),
        (r'\bQ\b', 'Q factor'),
        (r'\bslope\b', 'slope'),
        (r'\bangle\b', 'angle/degrees'),
        (r'\bdegrees?\b', 'degrees'),
        (r'\bphase\b', 'phase'),
        (r'\bchannels?\b', 'channels'),
        (r'\bbits?\b', 'bits'),
        (r'\border\b', 'order'),
        (r'\bcount\b', 'count'),
        (r'\bsize\b', 'size'),
        (r'\blength\b', 'length'),
    ]
    
    # Analyze each filter and parameter
    for filter_name, filter_data in data.get('filters', {}).items():
        for param_name, param_data in filter_data.get('parameters', {}).items():
            
            # Track FFmpeg types
            ftype = param_data.get('ffmpeg_type', 'unknown')
            ffmpeg_types[ftype].append((filter_name, param_name))
            
            # Analyze description for units
            desc = param_data.get('description', '').lower()
            for pattern, unit in unit_patterns:
                if re.search(pattern, desc, re.IGNORECASE):
                    detected_units[unit].append((filter_name, param_name, desc[:60]))
            
            # Analyze range patterns
            min_val = param_data.get('min')
            max_val = param_data.get('max')
            if min_val is not None and max_val is not None:
                # Categorize range
                if min_val == 0 and max_val == 1:
                    range_patterns['0-1 (normalized)'].append((filter_name, param_name))
                elif min_val == -1 and max_val == 1:
                    range_patterns['-1 to 1 (bipolar)'].append((filter_name, param_name))
                elif min_val == 0 and max_val == 100:
                    range_patterns['0-100 (percentage)'].append((filter_name, param_name))
                elif max_val == 20000 or max_val == 22050 or max_val == 24000:
                    range_patterns['audio frequency range'].append((filter_name, param_name))
                elif min_val < 0 and 'dB' in desc.lower():
                    range_patterns['negative dB (attenuation)'].append((filter_name, param_name))
                elif isinstance(min_val, float) or isinstance(max_val, float):
                    range_patterns['float range'].append((filter_name, param_name))
                else:
                    range_patterns['integer range'].append((filter_name, param_name))
            
            # Analyze default value patterns
            default = param_data.get('default')
            if default is not None:
                if isinstance(default, bool):
                    default_patterns['boolean'].append((filter_name, param_name))
                elif isinstance(default, str):
                    if re.match(r'^-?\d+\.?\d*$', str(default)):
                        default_patterns['numeric string'].append((filter_name, param_name))
                    else:
                        default_patterns['string/enum'].append((filter_name, param_name))
                elif isinstance(default, float):
                    default_patterns['float'].append((filter_name, param_name))
                elif isinstance(default, int):
                    default_patterns['integer'].append((filter_name, param_name))
    
    return {
        'ffmpeg_types': dict(ffmpeg_types),
        'detected_units': dict(detected_units),
        'range_patterns': dict(range_patterns),
        'default_patterns': dict(default_patterns)
    }


def print_report(analysis, data):
    """Print a formatted report."""
    
    print("=" * 70)
    print("FFMPEG AUDIO FILTER UNIT ANALYSIS")
    print(f"FFmpeg Version: {data.get('ffmpeg_version', 'unknown')}")
    print(f"Total Filters: {data.get('filter_count', 0)}")
    print("=" * 70)
    
    # FFmpeg Types
    print("\n## FFmpeg Parameter Types\n")
    print(f"{'Type':<20} {'Count':<8} Examples")
    print("-" * 70)
    for ftype, params in sorted(analysis['ffmpeg_types'].items(), key=lambda x: -len(x[1])):
        examples = ', '.join([f"{f}.{p}" for f, p in params[:3]])
        if len(params) > 3:
            examples += f" (+{len(params)-3} more)"
        print(f"{ftype:<20} {len(params):<8} {examples}")
    
    # Detected Units
    print("\n\n## Detected Unit Types (from descriptions)\n")
    print(f"{'Unit':<20} {'Count':<8} Example Parameters")
    print("-" * 70)
    for unit, params in sorted(analysis['detected_units'].items(), key=lambda x: -len(x[1])):
        examples = ', '.join([f"{f}.{p}" for f, p, _ in params[:3]])
        if len(params) > 3:
            examples += f" (+{len(params)-3} more)"
        print(f"{unit:<20} {len(params):<8} {examples}")
    
    # Range Patterns
    print("\n\n## Range Patterns\n")
    print(f"{'Pattern':<30} {'Count':<8}")
    print("-" * 50)
    for pattern, params in sorted(analysis['range_patterns'].items(), key=lambda x: -len(x[1])):
        print(f"{pattern:<30} {len(params):<8}")
    
    # Summary for automation
    print("\n\n## Automation Mapping Summary\n")
    print("""
Based on analysis, these mapping strategies are needed:

| Input Type          | FFmpeg Accepts        | Mapping Required           |
|---------------------|----------------------|----------------------------|
| 0-1 normalized      | 0-1                  | None (direct)              |
| 0-1 normalized      | 0-100                | multiply by 100            |
| 0-1 normalized      | min-max float        | linear interpolation       |
| 0-1 normalized      | frequency (Hz)       | logarithmic interpolation  |
| 0-1 normalized      | dB                   | linear (dB is perceptual)  |
| 0-1 normalized      | time (ms/s)          | linear or log (context)    |
| -1 to 1 (bipolar)   | centered range       | scale and offset           |
| boolean             | 0/1                  | threshold at 0.5           |
| enum/string         | specific values      | quantize to options        |
""")
    
    # Suggested curve types
    print("\n## Suggested Curve Types for Automation\n")
    print("""
| Parameter Context     | Curve Type   | Reason                              |
|-----------------------|--------------|-------------------------------------|
| Frequency, Hz, cutoff | Logarithmic  | Perceptually linear (octaves)       |
| dB, gain, level       | Linear       | Already perceptually scaled         |
| Time (attack/release) | Logarithmic  | Small values more critical          |
| Time (delay/duration) | Linear       | Usually linear perception           |
| Mix, wet/dry, blend   | Linear       | Direct percentage                   |
| Q factor              | Logarithmic  | Wide range, low values critical     |
| Ratio (compressor)    | Logarithmic  | 1:1 to 20:1 spans wide range        |
| Depth, width          | Linear       | Usually 0-1 or percentage           |
| Percentage (0-100)    | Linear       | Direct mapping                      |
""")


def export_unit_list(analysis):
    """Export a simple unit type list."""
    
    # Combine all unique unit indicators
    units = set()
    
    # From FFmpeg types
    units.update(analysis['ffmpeg_types'].keys())
    
    # From detected units
    units.update(analysis['detected_units'].keys())
    
    return sorted(units)


def main():
    input_file = sys.argv[1] if len(sys.argv) > 1 else "ffmpeg-audio-filters.json"
    
    try:
        with open(input_file, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"Error: File not found: {input_file}", file=sys.stderr)
        sys.exit(1)
    
    analysis = analyze_units(data)
    print_report(analysis, data)
    
    # Print simple unit list at the end
    print("\n" + "=" * 70)
    print("SIMPLE UNIT TYPE LIST")
    print("=" * 70)
    print("\nFFmpeg types:", ', '.join(sorted(analysis['ffmpeg_types'].keys())))
    print("\nDetected units:", ', '.join(sorted(analysis['detected_units'].keys())))
    
    # Optional: export to JSON
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
        with open(output_file, 'w') as f:
            json.dump({
                'ffmpeg_types': list(analysis['ffmpeg_types'].keys()),
                'detected_units': list(analysis['detected_units'].keys()),
                'full_analysis': {
                    'ffmpeg_types': {k: len(v) for k, v in analysis['ffmpeg_types'].items()},
                    'detected_units': {k: len(v) for k, v in analysis['detected_units'].items()},
                    'range_patterns': {k: len(v) for k, v in analysis['range_patterns'].items()},
                }
            }, f, indent=2)
        print(f"\nAnalysis exported to: {output_file}")


if __name__ == '__main__':
    main()
