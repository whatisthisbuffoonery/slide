# Editing course assets

JSON is used for both configuration and messages. It is ordinary UTF-8 text. Use double quotes, separate entries with commas, and avoid trailing commas.

For development, run windows/Slide.exe and edit windows/assets/. For a distributed copy, edit the assets folder next to that copy's executable. The dist folder is a build output; rebuilding copies the source assets into it.

## Message pool

Every course.json chooses its message file:

    "messages": "messages.json"

The pool contains any number of named messages:

```json
{
  "messages": [
    {
      "id": "work-rant",
      "label": "THE WORK RANT",
      "body": "This meeting could've been a nap.",
      "intended": "Besties",
      "actual": "Project Team",
      "consequence": "Your manager volunteers you to run the next meeting.",
      "avoided": "Your meeting review stays between friends."
    }
  ]
}
```

IDs must be unique within that course. Reordering the array does not change explicit obstacle references. Keep message bodies and consequences short enough to read during play; the UI wraps text but has fixed card/screen areas.

The visible card, readable nearby preview, defeat screen, and recap all use this same entry. You do not edit separate copies of the text.

An obstacle selects a specific message:

```json
{
  "position": [0, 0.75, 7],
  "amplitude": 3.5,
  "speed": 1.1,
  "phase": 0,
  "message": "work-rant"
}
```

Or set "message" to "*" to draw from the course pool. Wildcard draws are shuffled without replacement until the pool is exhausted. Selection stays fixed through checkpoint retries and F5 reloads with the same seed; a new journey changes the seed. Explicit IDs are useful for authored narrative sequences.

Only messages actually placed in the course appear as sent/avoided in its recap. An unused pool entry is not falsely marked as avoided.

## Audio

Each course config selects its own tracks:

```json
"audio": {
  "music": "audio/music.wav",
  "jump": "audio/jump.wav",
  "checkpoint": "audio/checkpoint.wav",
  "defeat": "audio/defeat.wav",
  "victory": "audio/victory.wav",
  "music_volume": 0.25,
  "effects_volume": 0.75
}
```

Paths resolve relative to that course's folder. Replace a file with the same name or change its path. Use null instead of a filename to disable that sound. Volumes range from 0 to 1.

Export **16-bit PCM WAV**, mono or stereo. Sample rates from 8 kHz to 192 kHz are resampled to 44.1 kHz at load time. PCM float, 24-bit WAV, MP3, and OGG need conversion first. Each input file is capped at 128 MiB, and each decoded track at roughly six minutes. Short looping music works best.

Music loops continuously; jump, checkpoint, defeat, and victory are one-shot effects. Loading a course switches its sound bank. Playback is independent of frame rendering. F5 reloads files and restarts the current track.

## Layout

- spawn / goal / checkpoints: [x, y, z] world positions.
- platforms: center, size, and RGB color in the 0..1 range. Center is the box centre, not its top surface.
- obstacles: position, horizontal oscillation amplitude, speed in radians/second, phase in radians, and message ID or "*".
- test_route and test_jumps: optional developer-only waypoints used by the traversal test; gameplay does not follow them.

There may be up to four active obstacles and three checkpoints per course. The message pool itself is not restricted to four entries.

## Add a course

1. Copy a course folder under assets/courses/.
2. Give course.json a unique id and a display name.
3. Edit its layout, message pool, and audio.
4. Add its folder path to assets/campaign.json.
5. Restart the game so it reloads the campaign list.

Example campaign:

```json
{
  "courses": [
    "courses/01-work-chat",
    "courses/02-personal-chat",
    "courses/03-group-chat"
  ]
}
```

F5 reloads the current course. It does not reload the campaign list mid-run. Content errors name the file or missing ID; correct the file and press F5 again.
