//! This file contains tests for the `stracciatella::vfs` module.

// Vfs tests.
mod vfs {
    #[test]
    fn read() {
        let (temp, dir) = create_temp_dir();
        create_data_slf(&dir); // data.slf

        let mut vfs = Vfs::new();
        vfs.add_slf(0, &dir.join("data.slf")).unwrap();
        let data = read_file_data(&vfs, "foo.txt");
        assert_eq!(&data, b"data.slf");

        temp.close().expect("close temp dir");
    }

    #[test]
    fn seek() {
        let (temp, dir) = create_temp_dir();
        create_data_slf(&dir); // data.slf

        let mut vfs = Vfs::new();
        vfs.add_slf(0, &dir.join("data.slf")).unwrap();
        let mut file = vfs.open(&Nfc::caseless_path("foo.txt")).unwrap();

        assert_eq!(file.seek(SeekFrom::End(0)).unwrap(), 8);
        assert_eq!(file.seek(SeekFrom::Current(4)).unwrap(), 12);
        assert_eq!(file.seek(SeekFrom::Current(-8)).unwrap(), 4);
        assert!(file.seek(SeekFrom::Current(-5)).is_err());
        assert_eq!(file.seek(SeekFrom::Start(0)).unwrap(), 0);

        temp.close().expect("close temp dir");
    }

    #[test]
    fn order() {
        let (temp, dir) = create_temp_dir();
        create_foo_slf(&dir); // foo.slf
        create_foobar_slf(&dir); // foobar.slf
        create_foo_bar_baz_txt(&dir); // baz.txt

        macro_rules! t {
            ($a: expr, $b: expr, $c: expr, $data: expr) => {
                let mut vfs = Vfs::new();
                vfs.add_dir($a, &dir).expect("dir");
                vfs.add_slf($b, &dir.join("foo.slf")).expect("foo");
                vfs.add_slf($c, &dir.join("foobar.slf")).expect("foobar");
                let data = read_file_data(&vfs, "foo/bar/baz.txt");
                assert_eq!(&data, $data);
            };
        }

        // integer order
        t!(0, 1, 2, b"baz.txt");
        t!(0, 2, 1, b"baz.txt");
        t!(1, 0, 2, b"foo.slf");
        t!(1, 2, 0, b"foobar.slf");
        t!(2, 0, 1, b"foo.slf");
        t!(2, 1, 0, b"foobar.slf");
        // when integer is the same, fifo order
        t!(0, 0, 0, b"baz.txt");
        t!(0, 0, 1, b"baz.txt");
        t!(0, 1, 0, b"baz.txt");
        t!(1, 0, 0, b"foo.slf");
        t!(1, 1, 0, b"foobar.slf");
        t!(1, 0, 1, b"foo.slf");
        t!(1, 1, 1, b"baz.txt");

        temp.close().expect("close temp dir");
    }

    #[test]
    fn paths() {
        let (temp, dir) = create_temp_dir();
        create_foo_slf(&dir); // foo.slf

        let mut vfs = Vfs::new();
        vfs.add_slf(0, &dir.join("foo.slf")).unwrap();
        // case insensitive
        let data = read_file_data(&vfs, "FOO/bar.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/BAR.TXT");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/BAR/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/bar/BAZ.TXT");
        assert_eq!(&data, b"foo.slf");
        // separators
        let data = read_file_data(&vfs, "foo/bar/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar/baz.txt");
        assert_eq!(&data, b"foo.slf");

        temp.close().expect("close temp dir");
    }

    // end of vfs tests
    //------------------

    use std::io::{Read, Seek, SeekFrom, Write};
    use std::path::{Path, PathBuf};

    use stracciatella::file_formats::slf::{SlfEntry, SlfEntryState, SlfHeader};
    use stracciatella::fs;
    use stracciatella::fs::{OpenOptions, TempDir};
    use stracciatella::unicode::Nfc;
    use stracciatella::vfs::Vfs;

    fn read_file_data(vfs: &Vfs, path: &str) -> Vec<u8> {
        let mut file = vfs.open(&Nfc::caseless_path(path)).expect("VfsFile");
        let mut data = Vec::new();
        file.read_to_end(&mut data).expect("Vec<u8>");
        data
    }

    /// The inner file data is the name.
    fn create_slf(dir: &Path, name: &str, library_path: &str, entry_paths: &[&str]) -> PathBuf {
        let header = SlfHeader {
            library_name: name.to_owned(),
            library_path: library_path.to_owned(),
            num_entries: entry_paths.len() as i32,
            ok_entries: entry_paths.len() as i32,
            sort: 0xFFFF,
            version: 0x200,
            contains_subdirectories: if library_path.is_empty() { 0 } else { 1 },
        };
        let path = dir.join(&name);
        let mut file = OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&path)
            .expect("open new file for writing");
        header.to_output(&mut file).expect("write header");
        let mut entries = entry_paths
            .iter()
            .map(|&entry_path| {
                let offset = file.seek(SeekFrom::Current(0)).expect("seek to entry data");
                let data = name.as_bytes();
                file.write_all(&data).expect("write entry data");
                SlfEntry {
                    file_path: entry_path.to_owned(),
                    offset: offset as u32,
                    length: data.len() as u32,
                    state: SlfEntryState::Ok,
                    file_time: 0,
                }
            })
            .collect::<Vec<SlfEntry>>();
        entries.sort_by(|a, b| a.file_path.cmp(&b.file_path));
        header
            .entries_to_output(&mut file, &entries)
            .expect("write entries");
        file.sync_all().expect("sync_all");
        path
    }

    /// The file data is "baz.txt".
    fn create_foo_bar_baz_txt(dir: &Path) {
        let dir = dir.join("foo/bar");
        let name = "baz.txt";
        fs::create_dir_all(&dir).expect("create_dir_all");
        fs::write(&dir.join(&name), name.as_bytes()).expect("write");
    }

    /// The inner file data is "foobar.slf".
    fn create_foobar_slf(dir: &Path) {
        create_slf(&dir, "foobar.slf", r"foo\bar\", &["baz.txt"]);
    }

    /// The inner file data is "foo.slf".
    fn create_foo_slf(dir: &Path) {
        create_slf(&dir, "foo.slf", "foo\\", &["bar.txt", "bar\\baz.txt"]);
    }

    /// The inner file data "data.slf".
    fn create_data_slf(dir: &Path) {
        create_slf(&dir, "data.slf", "", &["foo.txt"]);
    }

    /// The temporary dir and it's contents are removed when TempDir is closed or goes out of scope.
    fn create_temp_dir() -> (TempDir, PathBuf) {
        let temp = TempDir::new().expect("TempDir");
        let dir = temp.path().to_owned();
        (temp, dir)
    }
}
