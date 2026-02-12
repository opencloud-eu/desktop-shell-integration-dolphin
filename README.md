# OpenCloud Desktop Shell Integrations for Dolphin

This is the OpenCloud Desktop shell integration for the great KDE Dolphin in KDE Frameworks 6.

It adds overlay icons for local directories that are synced with `OpenCloud`.
The icons visualize the sync state of the files.

Also, a context menu item is added to Dolphins context menu for synced directories which
allows to access `OpenCloud` specific functions for the file item such as sharing.

## How to Install?

Best use the distro packages for your system, as it is important that the plugin is built against
the same Qt version that Dolphin is compiled against.

To compile yourself, follow the steps:

### Install the resources from their git repository

The resources are an mandatory dependency, they provide the icon files.

Follow the steps:
```
git clone https://github.com/opencloud-eu/desktop-shell-integration-resources

cd desktop-shell-integration-resources/
mkdir build && cd build/
cmake ..
```

### Build the dolphin overlay plugin:

```
git clone https://github.com/opencloud-eu/desktop-shell-integration-dolphin.git

cd desktop-shell-integration-dolphin/
mkdir build && cd build/
export OpenCloudShellResources_DIR=/path/above/desktop-shell-integration-resources/build/
cmake .. -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install
```

Restart Dophin to see the overlays.


