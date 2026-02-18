#!/usr/bin/env python3
"""
ffmpeg-filters-to-markdown.py
Converts FFmpeg audio filter JSON database to Markdown documentation

Usage: python3 ffmpeg-filters-to-markdown.py [input.json] [output.md]
       Default input:  ffmpeg-audio-filters.json
       Default output: ffmpeg-audio-filters.md
"""

import json
import sys
from datetime import datetime
from pathlib import Path


def bool_symbol(val):
    """Convert boolean to checkmark symbol."""
    return "✓" if val else "✗"


def format_range(min_val, max_val):
    """Format min/max range for display."""
    if min_val is not None and max_val is not None:
        return f"{min_val} → {max_val}"
    elif min_val is not None:
        return f"≥ {min_val}"
    elif max_val is not None:
        return f"≤ {max_val}"
    return "—"


def format_default(val):
    """Format default value for display."""
    if val is None:
        return "—"
    return f"`{val}`"


def escape_markdown(text):
    """Escape special markdown characters in table cells."""
    if not text:
        return ""
    return str(text).replace("|", "\\|").replace("\n", " ")


def generate_markdown(data):
    """Generate markdown documentation from filter data."""
    lines = []
    
    # Header
    lines.append("# FFmpeg Audio Filters Reference\n")
    lines.append(f"> **FFmpeg Version:** {data.get('ffmpeg_version', 'Unknown')}  ")
    lines.append(f"> **Generated:** {data.get('generated', datetime.now().isoformat())}  ")
    lines.append(f"> **Total Filters:** {data.get('filter_count', len(data.get('filters', {})))}\n")
    lines.append("---\n")
    
    # Table of Contents
    lines.append("## Table of Contents\n")
    filters = data.get('filters', {})
    for filter_name in sorted(filters.keys()):
        anchor = filter_name.lower().replace('_', '-')
        lines.append(f"- [{filter_name}](#{anchor})")
    lines.append("\n---\n")
    
    # Legend
    lines.append("""## Legend

| Symbol | Meaning |
|--------|---------|
| ✓ | Feature supported |
| ✗ | Feature not supported |
| 🎚️ | Parameter is automatable (can be changed at runtime) |

### Parameter Types

| Type | Description | Units/Notes |
|------|-------------|-------------|
| integer | Whole number | count, samples, bits |
| float | Decimal number | linear gain, ratio |
| boolean | true/false | 0/1 |
| string | Text value | expressions, filenames |
| duration | Time value | seconds, or HH:MM:SS.ms |
| flags | Bitfield | combined options |
| channel_layout | Channel config | stereo, 5.1, etc. |
| sample_format | Sample format | s16, s32, flt, dbl |

### Common Unit Conventions

| Suffix/Context | Unit | Example |
|----------------|------|---------|
| dB | Decibels | `volume=6dB` |
| Hz | Hertz (frequency) | `lowpass=f=1000` |
| ms | Milliseconds | `adelay=500` |
| % | Percentage | (rarely used directly) |

---

## Filters
""")
    
    # Process each filter
    for filter_name in sorted(filters.keys()):
        f = filters[filter_name]
        
        lines.append(f"\n---\n")
        lines.append(f"### {filter_name}\n")
        lines.append(f"{f.get('description', 'No description available.')}\n")
        
        # Properties table
        lines.append("| Property | Value |")
        lines.append("|----------|-------|")
        lines.append(f"| Timeline Support | {bool_symbol(f.get('timeline_support', False))} |")
        lines.append(f"| Slice Threading | {bool_symbol(f.get('slice_threading', False))} |")
        lines.append(f"| Command Support | {bool_symbol(f.get('command_support', False))} |")
        lines.append(f"| Inputs | {f.get('inputs', '?')} |")
        lines.append(f"| Outputs | {f.get('outputs', '?')} |")
        lines.append("")
        
        # Parameters
        params = f.get('parameters', {})
        if params:
            lines.append("#### Parameters\n")
            lines.append("| Parameter | Type | Range | Default | Auto | Description |")
            lines.append("|-----------|------|-------|---------|------|-------------|")
            
            for param_name, p in params.items():
                ptype = p.get('type', 'unknown')
                prange = format_range(p.get('min'), p.get('max'))
                pdefault = format_default(p.get('default'))
                pauto = "🎚️" if p.get('automatable', False) else "—"
                pdesc = escape_markdown(p.get('description', ''))
                
                lines.append(f"| `{param_name}` | {ptype} | {prange} | {pdefault} | {pauto} | {pdesc} |")
            
            lines.append("")
        else:
            lines.append("*This filter has no configurable parameters.*\n")
        
        # Example
        lines.append("**Basic Usage:**")
        lines.append("```bash")
        lines.append(f'ffmpeg -i input.wav -af "{filter_name}" output.wav')
        lines.append("```\n")
    
    # Footer with automation notes
    lines.append("""
---

## Automation & Modulation Notes

### Which Parameters Can Be Automated?

Parameters marked with 🎚️ support runtime changes via FFmpeg's command system. This is indicated by the 'T' flag in FFmpeg's help output.

### FFmpeg's sendcmd Filter

You can automate parameter changes over time using the `sendcmd` filter:

```bash
# Commands file format (cmds.txt):
# <time> <target> <command> <arg>
0.0 volume volume 0.5;
1.0 volume volume 1.0;
2.0 volume volume 0.25;

# Apply automation:
ffmpeg -i input.wav -af "sendcmd=f=cmds.txt,volume@v" output.wav
```

### Real-time Control via Named Pipes (Advanced)

```bash
# Create a named pipe
mkfifo /tmp/ffmpeg_cmds

# Run FFmpeg reading commands from pipe
ffmpeg -i input.wav -af "sendcmd=f=/tmp/ffmpeg_cmds,volume" output.wav &

# Send commands in real-time
echo "0 volume volume 0.5" > /tmp/ffmpeg_cmds
```

### Units and Value Formats

FFmpeg accepts values in their native units—no normalization needed:

| Parameter Type | Input Format | Example |
|----------------|--------------|---------|
| Volume/Gain | Linear or dB | `1.5` or `3dB` |
| Frequency | Hz | `1000` or `1k` |
| Time | Seconds or timestamp | `0.5` or `00:00:00.500` |
| Percentage | Decimal (0-1) | `0.75` for 75% |

### Building Your Own Automation

When building a GUI automation system:

1. **Store values in native units** (dB, Hz, etc.)—FFmpeg handles them directly
2. **Use the 'automatable' flag** from this database to determine which knobs to expose
3. **Generate sendcmd-compatible output** for time-based automation
4. **Consider the filter's command_support flag** for filters that support runtime changes

---

## Appendix: Automatable Parameters Quick Reference

""")
    
    # Generate quick reference of all automatable params
    lines.append("| Filter | Automatable Parameters |")
    lines.append("|--------|------------------------|")
    
    for filter_name in sorted(filters.keys()):
        f = filters[filter_name]
        params = f.get('parameters', {})
        auto_params = [name for name, p in params.items() if p.get('automatable', False)]
        
        if auto_params:
            lines.append(f"| {filter_name} | `{'`, `'.join(auto_params)}` |")
    
    lines.append("\n---\n")
    lines.append("*Generated by ffmpeg-filters-to-markdown.py*\n")
    
    return "\n".join(lines)


def main():
    input_file = sys.argv[1] if len(sys.argv) > 1 else "ffmpeg-audio-filters.json"
    output_file = sys.argv[2] if len(sys.argv) > 2 else "ffmpeg-audio-filters.md"
    
    input_path = Path(input_file)
    if not input_path.exists():
        print(f"Error: Input file not found: {input_file}", file=sys.stderr)
        sys.exit(1)
    
    print(f"Reading: {input_file}", file=sys.stderr)
    with open(input_path, 'r') as f:
        data = json.load(f)
    
    print(f"Processing {len(data.get('filters', {}))} filters...", file=sys.stderr)
    markdown = generate_markdown(data)
    
    output_path = Path(output_file)
    with open(output_path, 'w') as f:
        f.write(markdown)
    
    print(f"Written: {output_file}", file=sys.stderr)
    print(f"Total filters: {len(data.get('filters', {}))}", file=sys.stderr)


if __name__ == "__main__":
    main()
