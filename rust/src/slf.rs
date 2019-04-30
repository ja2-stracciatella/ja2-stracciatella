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

use std::io::ErrorKind::{InvalidData, InvalidInput};
use std::io::{Cursor, Error, Read, Result, Seek, SeekFrom, Write};

use std::time::{Duration, SystemTime, UNIX_EPOCH};

use byteorder::{ReadBytesExt, WriteBytesExt, LE};

// Number of bytes of the header in the library file.
pub const HEADER_BYTES: u32 = 532;

// Number of bytes of an entry in the library file.
pub const ENTRY_BYTES: u32 = 280;

// Header of the archive.
// The entries are at the end of the archive.
#[derive(Debug)]
pub struct SlfHeader {
    // Name of the library.
    //
    // Usually it's the name of the library file in uppercase.
    // Nul terminated string of 256 bytes, unused bytes are zeroed, unknown encoding (saw ASCII).
    pub library_name: String,

    // Base path of the files in the library.
    //
    // Empty or terminated by '\\'.
    // Nul terminated string of 256 bytes, unused bytes are zeroed, unknown encoding (saw ASCII).
    pub library_path: String,

    // Number of entries that are available.
    pub number_of_entries: i32,

    // Number of entries that have state Ok.
    pub used: i32,

    // TODO 0xFFFF probably means the entries are sorted by file path first, and by state second (Old < Ok)
    pub sort: u16,

    // TODO 0x0200 probably means v2.0
    pub version: u16,

    // TODO 0 when there are 0 '\\' characters in library_path (0 '\\' characters in the file names either, do they count?)
    //      1 when there is 1 '\\' character in library_path (0-2 '\\' characters in the file names)
    pub contains_subdirectories: u8,
}

// Entry of the archive.
#[derive(Debug)]
pub struct SlfEntry {
    // Path of the file from the library path.
    pub file_name: String,

    // Start offset of the file data in the library.
    pub offset: u32,

    // Length of the file data in the library.
    pub length: u32,

    // State of the entry.
    pub state: SlfEntryState,

    // FILETIME, the number of 10^-7 seconds (100-nanosecond intervals) from 1 Jan 1601.
    pub file_time: i64,
}

// State of an entry of the archive.
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum SlfEntryState {
    // Contains data and the data is up to date.
    //
    // Only entries with this state are used in the game.
    Ok,

    // TODO
    Deleted,

    // Contains data and the data is old.
    //
    // There should be an entry with the same path and state Ok next to this entry.
    Old,

    // TODO
    DoesNotExist,

    // Unknown state.
    Unknown(u8),
}

impl SlfHeader {
    // Read the header from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read + Seek,
    {
        input.seek(SeekFrom::Start(0))?;

        let mut handle = input.take(HEADER_BYTES as u64);
        let library_name = handle.read_fixed_string(256)?;
        let library_path = handle.read_fixed_string(256)?;
        let number_of_entries = handle.read_i32::<LE>()?;
        let used = handle.read_i32::<LE>()?;
        let sort = handle.read_u16::<LE>()?;
        let version = handle.read_u16::<LE>()?;
        let contains_subdirectories = handle.read_u8()?;
        handle.read_unused(7)?;
        assert_eq!(handle.limit(), 0);

        return Ok(Self {
            library_name,
            library_path,
            number_of_entries,
            used,
            sort,
            version,
            contains_subdirectories,
        });
    }

    // Write this header to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write + Seek,
    {
        let mut buffer = Vec::with_capacity(HEADER_BYTES as usize);
        let mut cursor = Cursor::new(&mut buffer);
        cursor.write_fixed_string(256, &self.library_name)?;
        cursor.write_fixed_string(256, &self.library_path)?;
        cursor.write_i32::<LE>(self.number_of_entries)?;
        cursor.write_i32::<LE>(self.used)?;
        cursor.write_u16::<LE>(self.sort)?;
        cursor.write_u16::<LE>(self.version)?;
        cursor.write_u8(self.contains_subdirectories)?;
        cursor.write_unused(7)?;
        assert_eq!(buffer.len(), HEADER_BYTES as usize);

        output.seek(SeekFrom::Start(0))?;
        output.write_all(&buffer)?;

        return Ok(());
    }

    // Read the entries from the input.
    #[allow(dead_code)]
    pub fn entries_from_input<T>(&self, input: &mut T) -> Result<Vec<SlfEntry>>
    where
        T: Read + Seek,
    {
        // TODO what should happen with a negative number of entries?
        if self.number_of_entries <= 0 {
            return Ok(Vec::new());
        }

        let num_entries = self.number_of_entries as u32;
        let num_bytes = num_entries * ENTRY_BYTES;
        input.seek(SeekFrom::End(-(num_bytes as i64)))?;

        let mut handle = input.take(num_bytes as u64);
        let mut entries = Vec::new();
        for _ in 0..num_entries {
            let file_name = handle.read_fixed_string(256)?;
            let offset = handle.read_u32::<LE>()?;
            let length = handle.read_u32::<LE>()?;
            let state: SlfEntryState = handle.read_u8()?.into();
            handle.read_unused(3)?;
            let file_time = handle.read_i64::<LE>()?;
            handle.read_unused(4)?;

            entries.push(SlfEntry {
                file_name,
                offset,
                length,
                state,
                file_time,
            });
        }
        assert_eq!(handle.limit(), 0);

        return Ok(entries);
    }

    // Write the entries to output.
    #[allow(dead_code)]
    pub fn entries_to_output<T>(&self, output: &mut T, entries: &[SlfEntry]) -> Result<()>
    where
        T: Write + Seek,
    {
        if self.number_of_entries < 0 || self.number_of_entries as usize != entries.len() {
            return Err(Error::new(
                InvalidInput,
                format!(
                    "unexpected number of entries {} != {}",
                    self.number_of_entries,
                    entries.len()
                ),
            ));
        }

        let num_bytes = self.number_of_entries as u32 * ENTRY_BYTES;
        let mut buffer = Vec::with_capacity(num_bytes as usize);
        let mut cursor = Cursor::new(&mut buffer);
        for entry in entries {
            cursor.write_fixed_string(256, &entry.file_name)?;
            cursor.write_u32::<LE>(entry.offset)?;
            cursor.write_u32::<LE>(entry.length)?;
            cursor.write_u8(entry.state.into())?;
            cursor.write_unused(3)?;
            cursor.write_i64::<LE>(entry.file_time)?;
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

        if end_of_data as u64 > output.seek(SeekFrom::End(-(num_bytes as i64)))? {
            // will increase the size of output
            output.seek(SeekFrom::Start(end_of_data as u64))?;
        }
        output.write_all(&buffer)?;

        return Ok(());
    }
}

impl SlfEntry {
    // Convert the file time of the entry to system time.
    #[allow(dead_code)]
    pub fn to_system_time(&self) -> Option<SystemTime> {
        // Unix epoch is 1 Jan 1970.
        // FILETIME is the number of 10^-7 seconds (100-nanosecond intervals) from 1 Jan 1601.
        const UNIX_EPOCH_AS_FILETIME: i64 = 11_644_473_600_000_000_0; // 100-nanoseconds

        if self.file_time < UNIX_EPOCH_AS_FILETIME {
            // TODO windows can also represent [0,UNIX_EPOCH_AS_FILETIME) but unix cannot, what should happen?
            // TODO file_time is signed, what to do with negative values?
            return None;
        }

        let unix = (self.file_time - UNIX_EPOCH_AS_FILETIME) as u64; // 100-nanoseconds
        let secs = unix / 1_000_000_0; //  seconds
        let nanos = (unix % 1_000_000_0) * 100; // nanoseconds
        return Some(UNIX_EPOCH + Duration::from_secs(secs) + Duration::from_nanos(nanos));
    }
}

impl From<SlfEntryState> for u8 {
    // All states map to a u8 value.
    fn from(state: SlfEntryState) -> Self {
        return match state {
            SlfEntryState::Ok => 0x00,
            SlfEntryState::Deleted => 0xFF,
            SlfEntryState::Old => 0x01,
            SlfEntryState::DoesNotExist => 0xFE,
            SlfEntryState::Unknown(value) => value,
        };
    }
}

impl From<u8> for SlfEntryState {
    // All u8 values map to a state.
    fn from(value: u8) -> Self {
        return match value {
            0x00 => SlfEntryState::Ok,
            0xFF => SlfEntryState::Deleted,
            0x01 => SlfEntryState::Old,
            0xFE => SlfEntryState::DoesNotExist,
            value => SlfEntryState::Unknown(value),
        };
    }
}

trait SlfReadExt: Read {
    // Reads and discards unused bytes.
    fn read_unused(&mut self, num_bytes: usize) -> Result<()> {
        let mut buffer = vec![0u8; num_bytes];
        self.read_exact(&mut buffer)?;
        return Ok(());
    }

    // Reads a nul terminated fixed size string.
    fn read_fixed_string(&mut self, num_bytes: usize) -> Result<String> {
        let mut buffer = vec![0u8; num_bytes];
        self.read_exact(&mut buffer)?;
        // must be nul terminated and valid utf8
        return match buffer.iter().position(|&byte| byte == 0) {
            Some(position) => match ::std::str::from_utf8(&buffer[..position]) {
                Ok(s) => Ok(s.to_string()),
                Err(e) => Err(Error::new(InvalidData, e)),
            },
            None => Err(Error::new(InvalidData, "string is not nul terminated")),
        };
    }
}

trait SlfWriteExt: Write {
    // Writes zeroed unused bytes.
    fn write_unused(&mut self, num_bytes: usize) -> Result<()> {
        let mut buffer = vec![0u8; num_bytes];
        self.write_all(&mut buffer)?;
        return Ok(());
    }

    // Write a nul terminated fixed size string, unused space is zeroed.
    fn write_fixed_string(&mut self, num_bytes: usize, string: &String) -> Result<()> {
        let mut buffer = vec![0u8; num_bytes];
        let string_bytes = string.as_bytes();
        if string_bytes.len() >= buffer.len() {
            return Err(Error::new(InvalidData, "string is too long"));
        }
        buffer[..string_bytes.len()].copy_from_slice(&string_bytes);
        self.write_all(&buffer)?;
        return Ok(());
    }
}

// Everything that implements Read gets SlfReadExt for free.
impl<T: Read + ?Sized> SlfReadExt for T {}

// Everything that implements Write gets SlfWriteExt for free.
impl<T: Write + ?Sized> SlfWriteExt for T {}
