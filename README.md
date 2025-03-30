# doomer

tsoding boomer -> coffeeispower woomer -> jeebuscrossaint doomer

I just wrote this program to see what my take on this small idea would look
like.

The shader embedded in the program is from coffeeispower on github under his
woomer repository. He was inspired by tsoding's boomer program and so was I too.

## Controls

| Control                                           | Description                                  |
| ------------------------------------------------- | -------------------------------------------- |
| Right Click or <kbd>ESC</kbd>                     | Quit the application.                        |
| Hold <kbd>CTRL</kbd>                              | Enable flashlight effect.                    |
| Drag with left mouse button                       | Move the image around.                       |
| Scroll wheel                                      | Zoom in/out.                                 |
| <kbd>Ctrl</kbd> + <kbd>SHIFT</kbd> + Scroll wheel | Change the radius of the flashlight.         |

## Building

On Nix just use nix build if you want it already has a flake.nix

The dependencies are in the flake.nix they are `raylib`, `gcc` (of course),
`grim`, `pkg-config`.

The final binary is only 20 kilobytes in size and internally it uses shellcode
of grim as a screenshot backend because I'm not reading all of those wayland
docs to take a screenshot, ok? Anyways grim I think is only 48 kilobytes anyways
as well, you got 68 kilobytes of ram I know it.

## Installing flake

put it in your flake.nix inputs

```nix
doomer.url = "github:jeebuscrossaint/doomer";
```

and then in like your configuration.

```nix
inputs.doomer.packages.${system}.default
```

## License

Licensed under the MIT License. Hefty credit to
[coffeeispower](https://github.com/coffeeispower/woomer) and
[tsoding](https://github.com/tsoding/boomer) for inspiration.
