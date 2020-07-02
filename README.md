# paoutput

PulseAudio output switcher, intended for use with dmenu.

## Usage

`$ paoutput <-g | -s [new_sink]>`
Options:
- `-g`: Get a newline-separated list of availabel sinks
- `-s [new_sink]`: Set all active streams to use the new sink. If the new sink is not given, it will be read from `stdin`.

## Example

```paoutput -g | dmenu | paoutput -s```

From my `~/.config/i3/config`:

```bindsym $mod+XF86AudioMute exec paoutput -g | rofi -dmenu | paoutput -s```
