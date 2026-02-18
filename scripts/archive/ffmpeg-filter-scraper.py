#!/usr/bin/env python3
"""
ffmpeg-filter-scraper.py
Extracts all audio filter parameters from FFmpeg and outputs clean JSON.

Usage: python3 ffmpeg-filter-scraper.py [output.json]
       Default output: ffmpeg-audio-filters.json

Requires: FFmpeg installed and in PATH
"""

import subprocess
import json
import re
import sys
from datetime import datetime, timezone

FFMPEG = "/usr/local/bin/ffmpeg"

def run_cmd(cmd):
    """Run a command and return stdout."""
    cmd = cmd.replace("ffmpeg", FFMPEG, 1)
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout + result.stderr


def get_ffmpeg_version():
    """Get FFmpeg version string."""
    output = run_cmd("ffmpeg -version")
    match = re.search(r'ffmpeg version (\S+)', output)
    return match.group(1) if match else "unknown"


def get_audio_filters():
    """Get list of audio filter names."""
    output = run_cmd("ffmpeg -filters 2>&1")
    filters = []
    
    for line in output.split('\n'):
        # Skip header lines
        if not line.strip() or line.startswith('Filters:') or '=' in line[:10]:
            continue
        
        # Match filter lines: flags, name, I/O type, description
        # Examples:
        #   " T.C volume           A->A       Change input volume."
        #   " ... abench            A->A       Benchmark part of a filtergraph."
        match = re.match(r'^\s*([T.][S.][C.]?)\s+(\w+)\s+(\S+)\s+(.*)', line)
        if match:
            flags, name, io_type, desc = match.groups()
            
            # Check if it's an audio filter (has A in I/O type)
            if 'A' in io_type:
                filters.append({
                    'name': name,
                    'flags': flags,
                    'io_type': io_type,
                    'brief_desc': desc.strip()
                })
    
    return filters


def parse_filter_help(filter_name):
    """Parse ffmpeg -h filter=<name> output."""
    output = run_cmd(f"ffmpeg -h filter={filter_name} 2>&1")
    
    result = {
        'description': '',
        'parameters': {}
    }
    
    # Get description (line after "Filter <name>")
    desc_match = re.search(rf'Filter {filter_name}\s*\n\s*(.+)', output)
    if desc_match:
        result['description'] = desc_match.group(1).strip()
    
    # Parse AVOptions section
    in_options = False
    for line in output.split('\n'):
        if 'AVOptions:' in line:
            in_options = True
            continue
        
        if not in_options:
            continue
        
        # Stop at next section (non-indented line that's not empty)
        if line and not line.startswith(' '):
            break
        
        # Match parameter lines
        # Format: "  name            <type>     ..F.A....T. description (from X to Y) (default Z)"
        param_match = re.match(
            r'^\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+<([^>]+)>\s+([.\w]+)\s+(.*)',
            line
        )
        
        if param_match:
            param_name = param_match.group(1)
            param_type = param_match.group(2)
            flags = param_match.group(3)
            rest = param_match.group(4)
            
            # Check if automatable (has T in flags)
            automatable = 'T' in flags
            
            # Extract range (from X to Y)
            min_val = None
            max_val = None
            range_match = re.search(r'\(from\s+(\S+)\s+to\s+(\S+)\)', rest)
            if range_match:
                min_str, max_str = range_match.groups()
                min_val = parse_number(min_str)
                max_val = parse_number(max_str)
            
            # Extract default
            default = None
            default_match = re.search(r'\(default\s+([^)]+)\)', rest)
            if default_match:
                default = parse_value(default_match.group(1).strip())
            
            # Extract description (before parentheses)
            desc = re.sub(r'\s*\(from\s.*', '', rest)
            desc = re.sub(r'\s*\(default\s.*', '', desc).strip()
            
            # Map FFmpeg types to our types
            type_map = {
                'int': 'integer', 'int64': 'integer', 'uint64': 'integer',
                'float': 'float', 'double': 'float',
                'boolean': 'boolean', 'bool': 'boolean',
                'string': 'string',
                'flags': 'flags',
                'duration': 'duration',
                'color': 'color',
                'rational': 'rational',
                'sample_fmt': 'sample_format',
                'pix_fmt': 'pixel_format',
                'channel_layout': 'channel_layout',
                'image_size': 'image_size',
                'video_rate': 'video_rate',
            }
            
            result['parameters'][param_name] = {
                'type': type_map.get(param_type, param_type),
                'ffmpeg_type': param_type,
                'description': desc,
                'min': min_val,
                'max': max_val,
                'default': default,
                'automatable': automatable
            }
    
    return result


def parse_number(s):
    """Parse a number, returning None for infinity/special values."""
    special = {
        'INF', 'inf', '-INF', '-inf',
        'FLT_MAX', '-FLT_MAX', 'DBL_MAX', '-DBL_MAX',
        'INT_MAX', '-INT_MAX', 'I64_MAX', '-I64_MAX',
        'INT_MIN', 'I64_MIN'
    }
    
    if s in special:
        return None
    
    try:
        if '.' in s or 'e' in s.lower():
            return float(s)
        return int(s)
    except ValueError:
        return None


def parse_value(s):
    """Parse a default value."""
    if s in ('null', 'NULL', ''):
        return None
    
    # Try as number first
    num = parse_number(s)
    if num is not None:
        return num
    
    # It's a string
    return s


def main():
    output_file = sys.argv[1] if len(sys.argv) > 1 else "ffmpeg-audio-filters.json"
    
    print(f"Getting FFmpeg version...", file=sys.stderr)
    version = get_ffmpeg_version()
    print(f"FFmpeg version: {version}", file=sys.stderr)
    
    print(f"Scanning audio filters...", file=sys.stderr)
    filters = get_audio_filters()
    print(f"Found {len(filters)} audio filters", file=sys.stderr)
    
    if not filters:
        print("ERROR: No audio filters found!", file=sys.stderr)
        print("Check that FFmpeg is installed and in PATH", file=sys.stderr)
        sys.exit(1)
    
    result = {
        'ffmpeg_version': version,
        'generated': datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
        'filter_count': len(filters),
        'filters': {}
    }
    
    for i, f in enumerate(filters, 1):
        name = f['name']
        print(f"\r  Processing: {name:20} ({i}/{len(filters)})   ", end='', file=sys.stderr)
        
        # Parse filter help
        help_data = parse_filter_help(name)
        
        # Determine capabilities from flags
        flags = f['flags']
        has_timeline = flags[0] == 'T'
        has_slice = flags[1] == 'S'
        has_command = len(flags) > 2 and flags[2] == 'C'
        
        # Count inputs/outputs from I/O type (e.g., "A->A", "AA->A", "A->N")
        io_type = f['io_type']
        io_match = re.match(r'([AN]+)->([AN]+)', io_type)
        if io_match:
            inputs = len(io_match.group(1))
            outputs = len(io_match.group(2))
        else:
            inputs = 1
            outputs = 1
        
        # Check for dynamic I/O
        if 'N' in io_type:
            inputs = 'dynamic' if io_type.startswith('N') else inputs
            outputs = 'dynamic' if io_type.endswith('N') else outputs
        
        result['filters'][name] = {
            'description': help_data['description'] or f['brief_desc'],
            'timeline_support': has_timeline,
            'slice_threading': has_slice,
            'command_support': has_command,
            'inputs': str(inputs),
            'outputs': str(outputs),
            'parameters': help_data['parameters']
        }
    
    print("\n", file=sys.stderr)
    
    # Write JSON
    with open(output_file, 'w') as fp:
        json.dump(result, fp, indent=2)
    
    print(f"Output: {output_file}", file=sys.stderr)
    print(f"Filters: {len(filters)}", file=sys.stderr)


if __name__ == '__main__':
    main()
