//! This file contains code to read and write SLF files.
//!
//! SLF is a file format that holds a collection of files and has the file extension `.slf`.
//!
//! I'm calling it "Sir-tech Library File" based on the name of STCI/STI.
//!
//!
//! # File Structure
//!
//! Based on "src/sgp/LibraryDatabase.cc", the file has the following structure:
//!
//!  * header - 532 bytes, always at the start of the file
//!  * data - any size, contains the data of the entries
//!  * entries - 280 bytes per entry, always at the end of the file
//!
//! Each entry represents a file.
//!
//! Numeric values are in little endian.
//!
//! Strings are '\0' terminated and have unused bytes zeroed.
//!
//! The paths are case-insensitive and use the '\\' character as a directory separator.
//! Probably the special names for current directory "." and parent directory ".." are not supported.
//! The header contains a library path, it is a path relative to the default directory (Data dir).
//! Each entry contains a file path, it is a path relative to the library path.
//! The encoding of the paths is unknown, but so far I've only seen ASCII.
//!
//!
//! # Header Structure
//!
//! Based on LIBHEADER in "src/sgp/LibraryDatabase.cc", the header has the following structure (532 bytes):
//!
//!  * 256 byte string with the library name
//!  * 256 byte string with the library path (empty or terminated by '\\', relative to Data dir)
//!  * 4 byte signed number with the total number of entries
//!  * 4 byte signed number with the total number of entries that have state FILE_OK 0x00
//!  * 2 byte unsigned number with name iSort (not used, only saw 0xFFFF, probably means it's sorted)
//!  * 2 byte unsigned number with name iVersion (not used, only saw 0x0200, probably means v2.0)
//!  * 1 byte unsigned number with name fContainsSubDirectories (not used, saw 0 and 1)
//!  * 3 byte padding (4 byte alignment)
//!  * 4 byte signed number with name iReserved (not used)
//!
//!
//! # Entry Structure
//!
//! Based on DIRENTRY in "src/sgp/LibraryDatabase.cc", the header has the following structure (280 bytes):
//!
//!  * 256 byte string with the file path (relative to the library path)
//!  * 4 byte unsigned number with the offset of the file data in the library file
//!  * 4 byte unsigned number with the length of the file data in the library file
//!  * 1 byte unsigned number with the state of the entry (saw FILE_OK 0x00 and FILE_OLD 0x01)
//!  * 1 byte unsigned number with name ubReserved (not used)
//!  * 2 byte padding (4 byte alignment)
//!  * 8 byte FILETIME (not used, from windows, the number of 10^-7 seconds (100-nanosecond intervals) from 1 Jan 1601)
//!  * 2 byte unsigned number with name usReserved2 (not used)
//!  * 2 byte padding (4 byte alignment)
//!

use std::io::ErrorKind::InvalidInput;
use std::io::{Cursor, Error, Read, Result, Seek, SeekFrom, Write};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use byteorder::{LE, ReadBytesExt, WriteBytesExt};

use crate::file_formats::{StracciatellaReadExt, StracciatellaWriteExt};

/// Number of bytes of the header in the library file.
pub const HEADER_BYTES: u32 = 532;

/// Number of bytes of an entry in the library file.
pub const ENTRY_BYTES: u32 = 280;

/// Unix epoch is 1 Jan 1970.
/// FILETIME is the number of 10^-7 seconds (100-nanosecond intervals) from 1 Jan 1601.
pub const UNIX_EPOCH_AS_FILETIME: u64 = 116_444_736_000_000_000; // 100-nanoseconds

/// Header of the archive.
/// The entries are at the end of the archive.
#[derive(Debug, Default, Eq, PartialEq)]
pub struct SlfHeader {
    /// Name of the library.
    ///
    /// Usually it's the name of the library file in uppercase.
    /// Nul terminated string of 256 bytes, unused bytes are zeroed, unknown encoding (saw ASCII).
    pub library_name: String,

    /// Base path of the files in the library.
    ///
    /// Empty or terminated by '\\'.
    /// Nul terminated string of 256 bytes, unused bytes are zeroed, unknown encoding (saw ASCII).
    pub library_path: String,

    /// Number of entries that are available.
    pub num_entries: i32,

    /// Number of entries that have state Ok and are used by the game.
    pub ok_entries: i32,

    /// TODO 0xFFFF probably means the entries are sorted by file path first, and by state second (Old < Ok)
    pub sort: u16,

    /// TODO 0x0200 probably means v2.0
    pub version: u16,

    /// TODO 0 when there are 0 '\\' characters in library_path (0 '\\' characters in the file names either, do they count?)
    ///      1 when there is 1 '\\' character in library_path (0-2 '\\' characters in the file names)
    pub contains_subdirectories: u8,
}

/// Entry of the archive.
#[derive(Debug, Default, Eq, PartialEq)]
pub struct SlfEntry {
    /// Path of the file from the library path.
    pub file_path: String,

    /// Start offset of the file data in the library.
    pub offset: u32,

    /// Length of the file data in the library.
    pub length: u32,

    /// State of the entry.
    pub state: SlfEntryState,

    /// FILETIME, the number of 10^-7 seconds (100-nanosecond intervals) from 1 Jan 1601.
    pub file_time: u64,
}

/// State of an entry of the archive.
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum SlfEntryState {
    /// Contains data and the data is up to date.
    ///
    /// Only entries with this state are used in the game.
    Ok,

    /// The default state, this entry is empty.
    ///
    /// Not used in the game, probably used in datalib98 for empty entries.
    Deleted,

    /// Contains data and the data is old.
    ///
    /// There should be an entry with the same path and state Ok next to this entry.
    Old,

    /// Not used here or in the game, probably used in datalib98 during path searches.
    DoesNotExist,

    // Unknown state.
    Unknown(u8),
}

impl SlfHeader {
    /// Read the header from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read + Seek,
    {
        input.seek(SeekFrom::Start(0))?;

        let mut handle = input.take(u64::from(HEADER_BYTES));
        let library_name = handle.read_fixed_string(256)?;
        let library_path = handle.read_fixed_string(256)?;
        let num_entries = handle.read_i32::<LE>()?;
        let ok_entries = handle.read_i32::<LE>()?;
        let sort = handle.read_u16::<LE>()?;
        let version = handle.read_u16::<LE>()?;
        let contains_subdirectories = handle.read_u8()?;
        handle.read_unused(7)?;
        assert_eq!(handle.limit(), 0);

        Ok(Self {
            library_name,
            library_path,
            num_entries,
            ok_entries,
            sort,
            version,
            contains_subdirectories,
        })
    }

    /// Write this header to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write + Seek,
    {
        let mut buffer = Vec::with_capacity(HEADER_BYTES as usize);
        let mut cursor = Cursor::new(&mut buffer);
        cursor.write_fixed_string(256, &self.library_name)?;
        cursor.write_fixed_string(256, &self.library_path)?;
        cursor.write_i32::<LE>(self.num_entries)?;
        cursor.write_i32::<LE>(self.ok_entries)?;
        cursor.write_u16::<LE>(self.sort)?;
        cursor.write_u16::<LE>(self.version)?;
        cursor.write_u8(self.contains_subdirectories)?;
        cursor.write_unused(7)?;
        assert_eq!(buffer.len(), HEADER_BYTES as usize);

        output.seek(SeekFrom::Start(0))?;
        output.write_all(&buffer)?;

        Ok(())
    }

    /// Read the entries from the input.
    #[allow(dead_code)]
    pub fn entries_from_input<T>(&self, input: &mut T) -> Result<Vec<SlfEntry>>
    where
        T: Read + Seek,
    {
        if self.num_entries <= 0 {
            return Err(Error::new(
                InvalidInput,
                format!("unexpected number of entries {}", self.num_entries),
            ));
        }

        let num_entries = self.num_entries as u32;
        let num_bytes = num_entries * ENTRY_BYTES;
        input.seek(SeekFrom::End(-(i64::from(num_bytes))))?;

        let mut handle = input.take(u64::from(num_bytes));
        let mut entries = Vec::new();
        for _ in 0..num_entries {
            let file_path = handle.read_fixed_string(256)?;
            let offset = handle.read_u32::<LE>()?;
            let length = handle.read_u32::<LE>()?;
            let state: SlfEntryState = handle.read_u8()?.into();
            handle.read_unused(3)?;
            let file_time = handle.read_u64::<LE>()?;
            handle.read_unused(4)?;

            entries.push(SlfEntry {
                file_path,
                offset,
                length,
                state,
                file_time,
            });
        }
        assert_eq!(handle.limit(), 0);

        Ok(entries)
    }

    /// Write the entries to output.
    #[allow(dead_code)]
    pub fn entries_to_output<T>(&self, output: &mut T, entries: &[SlfEntry]) -> Result<()>
    where
        T: Write + Seek,
    {
        if self.num_entries < 0 || self.num_entries as usize != entries.len() {
            return Err(Error::new(
                InvalidInput,
                format!(
                    "unexpected number of entries {} != {}",
                    self.num_entries,
                    entries.len()
                ),
            ));
        }

        let num_bytes = self.num_entries as u32 * ENTRY_BYTES;
        let mut buffer = Vec::with_capacity(num_bytes as usize);
        let mut cursor = Cursor::new(&mut buffer);
        for entry in entries {
            cursor.write_fixed_string(256, &entry.file_path)?;
            cursor.write_u32::<LE>(entry.offset)?;
            cursor.write_u32::<LE>(entry.length)?;
            cursor.write_u8(entry.state.into())?;
            cursor.write_unused(3)?;
            cursor.write_u64::<LE>(entry.file_time)?;
            cursor.write_unused(4)?;
        }
        assert_eq!(buffer.len(), num_bytes as usize);

        let mut end_of_data = HEADER_BYTES;
        for entry in entries {
            let end_of_entry = entry.offset + entry.length;
            if end_of_data < end_of_entry {
                end_of_data = end_of_entry;
            }
        }

        match output.seek(SeekFrom::End(-(i64::from(num_bytes)))) {
            Ok(position) if position >= u64::from(end_of_data) => {}
            _ => {
                // will increase the size of output
                output.seek(SeekFrom::Start(u64::from(end_of_data)))?;
            }
        }
        output.write_all(&buffer)?;

        Ok(())
    }
}

impl SlfEntry {
    /// Convert the file time of the entry to system time.
    #[allow(dead_code)]
    pub fn to_system_time(&self) -> Option<SystemTime> {
        if self.file_time < UNIX_EPOCH_AS_FILETIME {
            let n = UNIX_EPOCH_AS_FILETIME - self.file_time; // 100-nanoseconds
            let secs = Duration::from_secs(n / 10_000_000);
            let nanos = Duration::from_nanos((n % 10_000_000) * 100);
            UNIX_EPOCH
                .checked_sub(secs)
                .and_then(|x| x.checked_sub(nanos))
        } else {
            let n = self.file_time - UNIX_EPOCH_AS_FILETIME; // 100-nanoseconds
            let secs = Duration::from_secs(n / 10_000_000);
            let nanos = Duration::from_nanos((n % 10_000_000) * 100);
            UNIX_EPOCH
                .checked_add(secs)
                .and_then(|x| x.checked_add(nanos))
        }
    }

    /// Read the entry data from the input.
    #[allow(dead_code)]
    pub fn data_from_input<T>(&self, input: &mut T) -> Result<Vec<u8>>
    where
        T: Read + Seek,
    {
        input.seek(SeekFrom::Start(u64::from(self.offset)))?;

        let mut data = vec![0u8; self.length as usize];
        input.read_exact(&mut data)?;

        Ok(data)
    }

    /// Write the entry data to output.
    #[allow(dead_code)]
    pub fn data_to_output<T>(&self, output: &mut T, data: &[u8]) -> Result<()>
    where
        T: Write + Seek,
    {
        if self.offset < HEADER_BYTES {
            return Err(Error::new(
                InvalidInput,
                format!("unexpected data offset {}", self.offset),
            ));
        }
        if self.length as usize != data.len() {
            return Err(Error::new(
                InvalidInput,
                format!("unexpected data length {} != {}", self.length, data.len()),
            ));
        }

        output.seek(SeekFrom::Start(u64::from(self.offset)))?;
        output.write_all(data)?;

        Ok(())
    }
}

impl Default for SlfEntryState {
    /// Default value of SlfEntryState
    fn default() -> Self {
        SlfEntryState::Deleted
    }
}

impl From<SlfEntryState> for u8 {
    /// All states map to a u8 value.
    fn from(state: SlfEntryState) -> Self {
        match state {
            SlfEntryState::Ok => 0x00,
            SlfEntryState::Deleted => 0xFF,
            SlfEntryState::Old => 0x01,
            SlfEntryState::DoesNotExist => 0xFE,
            SlfEntryState::Unknown(value) => value,
        }
    }
}

impl From<u8> for SlfEntryState {
    /// All u8 values map to a state.
    fn from(value: u8) -> Self {
        match value {
            0x00 => SlfEntryState::Ok,
            0xFF => SlfEntryState::Deleted,
            0x01 => SlfEntryState::Old,
            0xFE => SlfEntryState::DoesNotExist,
            value => SlfEntryState::Unknown(value),
        }
    }
}

#[cfg(test)]
mod tests {
    use std::fmt::Debug;
    use std::io::Cursor;

    use crate::file_formats::slf::{
        ENTRY_BYTES, HEADER_BYTES, SlfEntry, SlfEntryState, SlfHeader, UNIX_EPOCH_AS_FILETIME,
    };

    #[inline]
    fn assert_ok<OK, ERR: Debug>(result: Result<OK, ERR>) -> OK {
        assert!(result.is_ok());
        result.unwrap()
    }

    #[test]
    fn write_and_read_in_memory() {
        let test_header = SlfHeader {
            library_name: "test library".to_string(),
            library_path: "libdir\\".to_string(),
            num_entries: 1,
            ok_entries: 1,
            sort: 0xFFFF,
            version: 0x0200,
            contains_subdirectories: 1,
        };
        let test_data = b"file contents\n".to_vec();
        let test_data_len = test_data.len() as u32;
        let test_entries = vec![SlfEntry {
            file_path: "file.ext".to_string(),
            offset: HEADER_BYTES,
            length: test_data_len,
            state: SlfEntryState::Ok,
            file_time: UNIX_EPOCH_AS_FILETIME,
        }];
        let after_header_pos = u64::from(HEADER_BYTES);
        let after_data_pos = after_header_pos + u64::from(test_data_len);
        let after_entries_pos = after_data_pos + u64::from(ENTRY_BYTES);
        let mut buf: Vec<u8> = Vec::new();
        let mut f = Cursor::new(&mut buf);

        // write
        assert_ok(test_header.to_output(&mut f));
        assert_eq!(f.position(), after_header_pos);
        for entry in &test_entries {
            let after_entry_data_pos = u64::from(entry.offset + entry.length);
            assert_ok(entry.data_to_output(&mut f, &test_data));
            assert_eq!(f.position(), after_entry_data_pos);
        }
        assert_ok(test_header.entries_to_output(&mut f, &test_entries));
        assert_eq!(f.position(), after_entries_pos);

        // read
        let header = assert_ok(SlfHeader::from_input(&mut f));
        assert_eq!(f.position(), after_header_pos);
        assert_eq!(test_header, header);
        let entries = assert_ok(header.entries_from_input(&mut f));
        assert_eq!(f.position(), after_entries_pos);
        assert_eq!(test_entries, entries);
        for entry in &entries {
            let after_entry_data_pos = u64::from(entry.offset + entry.length);
            let data = assert_ok(entry.data_from_input(&mut f));
            assert_eq!(f.position(), after_entry_data_pos);
            assert_eq!(test_data, data);
        }
    }
}
