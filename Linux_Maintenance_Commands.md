# Linux Maintenance Commands Reference
## Host Machine (Falcor) & VirtualBox VM (TallerV)
### Context: Linux Mint (MATE / XFCE), Ubuntu 22.04 LTS base (jammy)

---

## 1. DISK ANALYSIS

### Overview of all filesystems
```bash
df -h                          # Human-readable disk usage per filesystem
lsblk                          # Physical block device tree (drives, partitions, mounts)
mount | grep var               # Check if /var is a separate partition
```

### Find space consumers
```bash
# Top consumers at any directory level
du -sh /path/* 2>/dev/null | sort -rh | head -20

# Specific depth scan (useful for /var)
sudo du -h /var --max-depth=2 | sort -rh | head -30

# Full home folder top consumers
du -sh ~//* 2>/dev/null | sort -rh | head -20
```

### Check specific partition
```bash
df -h /var                     # Check only /var partition
df -h /                        # Check only root partition
df -h /boot                    # Check only boot partition
```

---

## 2. PACKAGE MANAGEMENT

### List and inspect packages
```bash
dpkg -l                                          # List all installed packages
dpkg -l | grep -i packagename                    # Search for specific package
dpkg -l | grep -iE "pattern1|pattern2"          # Search multiple patterns
apt-mark showmanual                              # Show manually installed packages
apt list --upgradable 2>/dev/null                # List upgradable packages
apt list --upgradable 2>/dev/null | head -20     # First 20 upgradable packages
```

### Update and upgrade
```bash
sudo apt update                                  # Refresh package lists
sudo apt upgrade                                 # Upgrade all packages (conservative)
sudo apt full-upgrade                            # Upgrade including dependency changes
sudo apt update && sudo apt upgrade              # Update then upgrade in one line
```

### Remove packages
```bash
sudo apt remove packagename                      # Remove package (keep config files)
sudo apt purge packagename                       # Remove package AND config files
sudo apt purge pkg1 pkg2 pkg3                    # Purge multiple packages at once
sudo apt autoremove                              # Remove orphaned auto-installed packages
sudo apt autoremove --purge                      # Remove orphaned packages + their configs
sudo apt clean                                   # Remove cached .deb installer files
```

### Find orphaned packages
```bash
sudo apt-get install deborphan                   # Install orphan finder
deborphan                                        # List orphaned libraries
```

---

## 3. KERNEL MANAGEMENT

### Inspect kernels
```bash
uname -r                                         # Currently running kernel
dpkg -l | grep linux-image                       # All installed kernel images
dpkg -l | grep "5.15.0-76"                       # All packages for specific kernel version
```

### Remove old kernels
```bash
# Remove kernel image and all associated packages
sudo apt purge linux-image-X.X.X-XX-generic \
              linux-modules-X.X.X-XX-generic \
              linux-modules-extra-X.X.X-XX-generic \
              linux-headers-X.X.X-XX \
              linux-headers-X.X.X-XX-generic

# Clean ghost rc entries (removed but config remaining)
sudo dpkg --purge $(dpkg -l | awk '/^rc.*linux/{print $2}')

# Follow up with autoremove
sudo apt autoremove --purge
```

---

## 4. DOCKER CLEANUP & REMOVAL

### Inspect Docker usage
```bash
docker images                                    # List all images with sizes
docker ps -a                                     # List all containers (including stopped)
```

### Clean Docker data
```bash
docker system prune -a                           # Remove all unused images, containers, cache
```

### Remove Docker completely
```bash
# Find all Docker packages
dpkg -l | grep -i docker

# Purge all Docker packages
sudo apt purge docker-ce docker-ce-cli docker-ce-rootless-extras \
             docker-buildx-plugin docker-compose-plugin

sudo apt autoremove --purge

# Remove Docker APT repository
sudo rm /etc/apt/sources.list.d/docker.list

# Check for orphaned GPG key
ls /etc/apt/trusted.gpg.d/ | grep -i docker
```

---

## 5. FLATPAK CLEANUP & REMOVAL

### Inspect Flatpak
```bash
flatpak list                                     # List installed apps and runtimes
flatpak --version                                # Check symlink handling (--symlinks is opt-in)
```

### Remove Flatpak apps
```bash
sudo flatpak uninstall com.app.Name --delete-data   # Remove app and its data
flatpak uninstall --unused                           # Remove orphaned runtimes
```

---

## 6. APT REPOSITORY MANAGEMENT

### Inspect repositories
```bash
ls /etc/apt/sources.list.d/                      # List all third-party repo files
cat /etc/apt/sources.list.d/reponame.list        # View specific repo config
ls /etc/apt/trusted.gpg.d/                       # List GPG signing keys
```

### Clean up repositories
```bash
sudo rm /etc/apt/sources.list.d/reponame.list    # Remove specific repo
sudo rm /etc/apt/trusted.gpg.d/keyname.gpg       # Remove specific GPG key
sudo apt update                                   # Refresh after repo changes
```

---

## 7. JOURNAL LOG MANAGEMENT

### Inspect journal
```bash
journalctl --disk-usage                          # Current journal disk usage
du -sh /var/log/journal/                         # Journal directory size
```

### Trim journal (one-time)
```bash
sudo journalctl --vacuum-size=100M               # Trim to 100MB (host machine)
sudo journalctl --vacuum-size=50M                # Trim to 50MB (VM)
sudo journalctl --vacuum-time=2weeks             # Remove entries older than 2 weeks
```

### Set permanent journal size limit
```bash
sudo nano /etc/systemd/journald.conf
# Find line: #SystemMaxUse=
# Change to: SystemMaxUse=100M  (host) or SystemMaxUse=50M  (VM)

sudo systemctl restart systemd-journald          # Apply changes immediately
```

---

## 8. DESKTOP ENVIRONMENT AUDIT

### Check display manager
```bash
systemctl status display-manager                 # Which display manager is running
```

### Find active desktop themes
```bash
xfconf-query -c xsettings -p /Net/IconThemeName  # Active icon theme (XFCE)
xfconf-query -c xsettings -p /Net/ThemeName      # Active GTK theme (XFCE)
```

### List installed themes and icons
```bash
ls /usr/share/icons/                             # All icon themes
ls /usr/share/themes/                            # All GTK themes
du -sh /usr/share/icons/* | sort -rh | head -10  # Icon themes by size
du -sh /usr/share/themes/* | sort -rh | head -10 # GTK themes by size
```

### Remove unused themes (keep active + system essentials)
```bash
# System icon essentials to KEEP: hicolor, Adwaita, HighContrast, locolor, default
sudo rm -rf /usr/share/icons/ThemeToRemove

# GTK theme essentials to KEEP: Adwaita, Adwaita-dark, Default, Default-hdpi,
#                                Default-xhdpi, HighContrast, Raleigh
sudo rm -rf /usr/share/themes/ThemeToRemove
```

---

## 9. LOCALE MANAGEMENT

### Inspect locales
```bash
ls /usr/share/locale/                            # All locale directories
du -sh /usr/share/locale/                        # Total locale size
locale                                           # Currently active locale settings
```

### Clean unused locales
```bash
sudo apt install localepurge
# Select locales to KEEP during interactive setup (e.g. en, es_CO)

# If USE_DPKG is enabled, disable it for immediate purge:
sudo sed -i 's/^USE_DPKG/#USE_DPKG/' /etc/locale.nopurge
sudo localepurge                                 # Run the purge
cat /etc/locale.nopurge                          # Verify configuration
```

---

## 10. SERVICE MANAGEMENT

### Inspect services
```bash
systemctl list-units --type=service --state=running    # Currently running services
systemctl list-units --type=service --state=enabled    # Services enabled at boot
systemctl status servicename                            # Status of specific service
```

### Disable unnecessary services
```bash
# Disable and stop immediately with --now
sudo systemctl disable --now servicename

# Services safe to disable on a VirtualBox VM:
sudo systemctl disable --now \
    ModemManager \        # Mobile modem manager
    avahi-daemon \        # mDNS network discovery
    cups \                # Printing
    cups-browsed \        # Remote printer discovery
    colord \              # Color profile management
    kerneloops \          # Kernel crash reporter
    zfs-zed \             # ZFS event daemon
    wpa_supplicant \      # WiFi manager
    packagekit \          # Background package daemon
    upower                # Power/battery management
```

### Re-enable a service if needed
```bash
sudo systemctl enable --now servicename
```

---

## 11. WIFI POWER MANAGEMENT FIX

### Diagnose WiFi issues after kernel upgrade
```bash
dmesg | grep -i iwlwifi | tail -20               # WiFi driver messages
cat /etc/NetworkManager/conf.d/*.conf            # Check power management config
```

### Fix WiFi disconnection on Intel chips
```bash
# wifi.powersave = 3 means enabled (causes disconnections on older hardware)
# wifi.powersave = 2 means disabled (stable)
ls /etc/NetworkManager/conf.d/
sudo nano /etc/NetworkManager/conf.d/filename.conf
# Change: wifi.powersave = 3  →  wifi.powersave = 2

sudo systemctl restart NetworkManager            # Apply immediately
```

---

## 12. DUPLICATE FILE DETECTION

### Install and use fdupes
```bash
sudo apt install fdupes

# Safe read-only scan with summary (no deletions)
fdupes -r --summarize /path/to/scan

# Check symlink handling (important before scanning)
fdupes --help | grep -i sym
# If --symlinks is opt-in (default safe), no flag needed
# Never scan symlinks and their targets together

# Analyze duplicate file types
fdupes -r /path | grep -v "^$" | grep -oE "\.[a-zA-Z0-9]+" | sort | uniq -c | sort -rn | head -20

# Save duplicate list to file for review
fdupes -r /path > /tmp/duplicates.txt
wc -l /tmp/duplicates.txt
cat /tmp/duplicates.txt | head -40

# Print only redundant copies (omits first occurrence of each set)
fdupes -r -f /path > /tmp/to_delete.txt

# Interactive deletion mode (choose which copy to keep per set)
fdupes -r -d /path
# In interactive mode: type number to keep, 0 to skip set, Ctrl+C to abort
```

---

## 13. VIRTUALBOX VM OPTIMIZATION (HOST MACHINE)

### Locate VM files
```bash
find /path -name "*.vdi" 2>/dev/null             # Find virtual disk files
find /path -name "*.ova" 2>/dev/null             # Find exported appliances
ls -lh /path/to/machine.vdi                      # Check .vdi file size
```

### Zero-fill free space (INSIDE the VM before compaction)
```bash
# Unmount shared folder first
sudo umount ~/shared

# Fill all free space with zeros
# "No space left on device" = SUCCESS, not an error
sudo dd if=/dev/zero of=/tmp/zero.tmp bs=4M status=progress; sudo rm /tmp/zero.tmp

# Shut down cleanly
sudo shutdown -h now
```

### Compact virtual disk (ON HOST after VM shutdown)
```bash
VBoxManage modifymedium --compact /path/to/machine.vdi
ls -lh /path/to/machine.vdi                      # Verify size reduction
```

### Export to .ova format (ON HOST)
```bash
VBoxManage export VMname -o /path/to/output.ova --options manifest
# --options manifest adds integrity checksum for students importing the VM
```

### List available VMs
```bash
VBoxManage list vms                              # All registered VMs
VBoxManage list runningvms                       # Currently running VMs
```

---

## 14. GENERAL USEFUL COMMANDS

### File and directory operations
```bash
ls -lh /path/                                    # List with human-readable sizes
du -sh /path/                                    # Total size of directory
find /path -name "pattern" 2>/dev/null           # Find files by name
rm -rf /path/to/directory                        # Remove directory and contents
```

### System information
```bash
uname -r                                         # Kernel version
uname -a                                         # Full system information
lspci | grep -i network                          # Network hardware
lspci | grep -i vga                              # Graphics hardware
free -h                                          # RAM usage
top / htop                                       # Process monitor
```

### APT source management
```bash
ls /etc/apt/sources.list.d/                      # Third-party repositories
cat /etc/apt/sources.list                        # Main repository list
ls /etc/apt/trusted.gpg.d/                       # GPG signing keys
```

### Bash tricks used in this session
```bash
# Run second command only if first succeeds
command1 && command2

# Run second command regardless of first result  
command1 ; command2

# Suppress errors and show human message
ls /path 2>/dev/null && echo "exists" || echo "gone"

# Filter multiple patterns with grep
grep -iE "pattern1|pattern2|pattern3" file

# Extract file extensions and count occurrences
find /path | grep -oE "\.[a-zA-Z0-9]+" | sort | uniq -c | sort -rn

# Remove block of text between markers in a file
sed -i '/start_marker/,/end_marker/d' file

# Comment out a line in a config file
sed -i 's/^OPTION/#OPTION/' /etc/config/file

# List only manually installed packages matching pattern
apt-mark showmanual | grep -iE "pattern"

# Purge all rc (removed but config remaining) linux packages
sudo dpkg --purge $(dpkg -l | awk '/^rc.*linux/{print $2}')
```

---

## 15. MAINTENANCE SCHEDULE RECOMMENDATIONS

### Monthly (5 minutes)
```bash
df -h                                            # Check partition health
sudo apt update && sudo apt upgrade              # Apply updates
sudo apt autoremove --purge                      # Clean orphans
sudo apt clean                                   # Clean package cache
```

### Per semester (before VM distribution)
1. Run full software audit inside VM
2. Remove unused applications and themes
3. Update all packages
4. Disable unnecessary services
5. Zero-fill free space
6. Compact `.vdi` on host
7. Export fresh `.ova`
8. Test import on a clean machine before distributing

### Warning thresholds
- **> 80% partition usage** — attention needed soon
- **> 90% partition usage** — immediate action required
- **> 95% partition usage** — critical, system instability possible
