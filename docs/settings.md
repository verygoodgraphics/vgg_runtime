The VGG Daruma file contains a `settings.json` file with the following contents:

```
{
  "launchFrameId": "xxx",
  "currentTheme": "light",
  "themes": [
    "dark",
    "light"
  ],
  "breakpoints": [
    {
      "minWidth": 0,
      "dark": "frame-id-when-the-viewport-width-is-greater-than-or-equal-to-0-in-dark-theme",
      "light": "frame-id-when-the-viewport-width-is-greater-than-or-equal-to-0-in-light-theme"
    },
    {
      "minWidth": 768,
      "dark": "frame-id-when-the-viewport-width-is-greater-than-or-equal-to-768-in-dark-theme",
      "light": "frame-id-when-the-viewport-width-is-greater-than-or-equal-to-768-in-light-theme"
    }
  ]
}
```

|Field|Description|
|-|-|
| launchFrameId | Which frame will be displayed when a valid frame ID is not found in `breakpoints` |
| currentTheme | Current theme name |
| themes | Theme names array |
| breakpoints | Breakpoints array  |

The logic for selecting which frame to display when VGG loads a Daruma file is as follows:
1. Find the corresponding breakpoint configuration based on the width of the current viewport.
2. According to the current theme, locate the corresponding frame ID in the breakpoint configuration.
3. If the frame ID is not found, use the launch frame ID.
4. If the launch frame ID is not found or not specified, display the first frame.