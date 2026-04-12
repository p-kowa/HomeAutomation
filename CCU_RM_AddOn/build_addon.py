import tarfile
import os
import io

src_dir = r"C:\Daten\Arduino\PlatformIO\Projects\HB-UNI-Sensor1_BME280_E-Paper\CCU_RM\src"
output  = r"C:\Daten\Arduino\PlatformIO\Projects\HB-UNI-Sensor1_BME280_E-Paper\CCU_RM\hb-pk-devices-addon.tgz"

executables = {
    'update_script',
    'install_hb-uni-sensor-THPL-BME280',
    'uninstall_hb-uni-sensor-THPL-BME280',
    'patchworker',
    'update-check.cgi',
    'hb-pk-devices-addon',
}

# Binary file extensions — do NOT convert line endings
binary_exts = {'.png', '.jpg', '.gif', '.tgz', '.gz', '.zip'}

with tarfile.open(output, "w:gz") as tar:
    for root, dirs, files in os.walk(src_dir):
        dirs.sort()
        for fname in sorted(files):
            if 'untitled.txt' in fname:
                continue
            fpath = os.path.join(root, fname)
            arcname = "./" + os.path.relpath(fpath, src_dir).replace("\\", "/")
            info = tar.gettarinfo(fpath, arcname=arcname)

            ext = os.path.splitext(fname)[1].lower()
            is_binary = ext in binary_exts

            if fname in executables:
                info.mode = 0o755
                print(f"[exec 755] {arcname}")
            else:
                info.mode = 0o644

            if is_binary:
                with open(fpath, 'rb') as f:
                    tar.addfile(info, f)
            else:
                # Convert CRLF -> LF
                raw = open(fpath, 'rb').read()
                converted = raw.replace(b'\r\n', b'\n')
                if raw != converted:
                    print(f"  [CRLF->LF] {arcname}")
                info.size = len(converted)
                tar.addfile(info, io.BytesIO(converted))

print(f"\nDone: {output}")
