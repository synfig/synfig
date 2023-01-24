# Snap useful commands

Clean snap builder:
\
`snapcraft clean --use-lxd`

Build snap using lxc containers with debug messages:
\
`snapcraft --use-lxd --debug`

Mount snap image to folder:
\
`mkdir ./snap-mount && sudo mount -t squashfs -o ro ./synfigstudio.snap ./snap-mount/`

Install snap for local testing:
\
`snap install --dangerous ./synfigstudio.snap`

Login to launchpad (required for remote build):
\
`snapcraft login`

Build using launchpad builder (allow for ARM builds):
\
`snapcraft remote-build`

Upload artifacts to Ubuntu store (candidate channel):
\
`snapcraft upload --release=candidate ./synfigstudio.snap`