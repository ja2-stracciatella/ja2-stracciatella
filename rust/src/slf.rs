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

use std::io::Cursor;
use std::io::Error;
use std::io::ErrorKind::InvalidData;
use std::io::Read;
use std::io::Result;
use std::io::SeekFrom;
use std::io::Write;

use std::mem::size_of;

use std::time::Duration;
use std::time::SystemTime;
use std::time::UNIX_EPOCH;

use byteorder::LittleEndian;
use byteorder::ReadBytesExt; // extends ::std::io::Read
use byteorder::WriteBytesExt; // extends ::std::io::Write

// File representation of SlfHeader.
#[repr(C, packed)]
#[allow(non_snake_case)]
struct LIBHEADER {
    pub sLibName: [u8; 256],
    pub sPathToLibrary: [u8; 256],
    pub iEntries: i32,
    pub iUsed: i32,
    pub iSort: u16,
    pub iVersion: u16,
    pub fContainsSubDirectories: i32,
    pub iReserved: i32,
}

// File representation of SlfEntry.
#[repr(C, packed)]
#[allow(non_snake_case)]
struct DIRENTRY {
    pub sFileName: [u8; 256],
    pub uiOffset: u32,
    pub uiLength: u32,
    pub ubState: u8,
    pub ubReserved: u8,
    _padding: [u8; 2], //< structure is aligned to 4 bytes
    pub sFileTime: i64,
    pub usReserved2: u16,
    _padding2: [u8; 2], //< structure is aligned to 4 bytes
}

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
#[derive(Debug)]
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

// Data starts after the header.
#[allow(dead_code)]
pub fn start_of_data() -> SeekFrom {
    return SeekFrom::Start(size_of::<LIBHEADER>() as u64);
}

// Entries are at the end fo the file.
#[allow(dead_code)]
pub fn start_of_entries(num_entries: i32) -> SeekFrom {
    if num_entries <= 0 {
        return SeekFrom::End(0);
    }
    return SeekFrom::End(-(num_entries as i64 * size_of::<DIRENTRY>() as i64));
}

impl SlfHeader {
    // Create an archive header from the input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        const NUM_BYTES: usize = size_of::<LIBHEADER>();
        debug_assert_eq!(NUM_BYTES, 532);

        // use local buffer
        let mut buffer: [u8; NUM_BYTES] = [0u8; NUM_BYTES];
        input.read_exact(&mut buffer)?;
        let mut cursor = Cursor::new(&buffer[..]);

        // read data
        let library_name = read_string(&mut cursor, 256)?;
        let library_path = read_string(&mut cursor, 256)?;
        let number_of_entries = read_i32(&mut cursor)?;
        let used = read_i32(&mut cursor)?;
        let sort = read_u16(&mut cursor)?;
        let version = read_u16(&mut cursor)?;
        let contains_subdirectories = read_u8(&mut cursor)?;
        read_unused(&mut cursor, 7)?;
        debug_assert_eq!(cursor.position() as usize, NUM_BYTES);

        return Ok(SlfHeader {
            library_name,
            library_path,
            number_of_entries,
            used,
            sort,
            version,
            contains_subdirectories,
        });
    }

    // Write the archive header to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<&Self>
    where
        T: Write,
    {
        const NUM_BYTES: usize = size_of::<LIBHEADER>();
        debug_assert_eq!(NUM_BYTES, 532);

        // use local buffer
        let mut buffer: [u8; NUM_BYTES] = [0u8; NUM_BYTES];
        let mut cursor = Cursor::new(&mut buffer[..]);

        // write to buffer
        write_string(&mut cursor, 256, &self.library_name)?;
        write_string(&mut cursor, 256, &self.library_path)?;
        write_i32(&mut cursor, self.number_of_entries)?;
        write_i32(&mut cursor, self.used)?;
        write_u16(&mut cursor, self.sort)?;
        write_u16(&mut cursor, self.version)?;
        write_u8(&mut cursor, self.contains_subdirectories)?;
        write_unused(&mut cursor, 7)?;
        debug_assert_eq!(cursor.position() as usize, NUM_BYTES);

        // write data
        output.write_all(&buffer)?;

        return Ok(&self);
    }

    // Read the entries from the input.
    // The input must be in the correct position.
    #[allow(dead_code)]
    pub fn read_entries_from<T>(&self, input: &mut T) -> Result<Vec<SlfEntry>>
    where
        T: Read,
    {
        // TODO use local buffer
        let mut entries: Vec<SlfEntry> = Vec::new();
        for _ in 0..self.number_of_entries {
            entries.push(SlfEntry::from_input(input)?);
        }
        return Ok(entries);
    }
}

impl SlfEntry {
    // Create an archive entry from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        const NUM_BYTES: usize = size_of::<DIRENTRY>();
        debug_assert_eq!(NUM_BYTES, 280);

        // use local buffer
        let mut buffer: [u8; NUM_BYTES] = [0u8; NUM_BYTES];
        input.read_exact(&mut buffer)?;
        let mut cursor = Cursor::new(&buffer[..]);

        // read data
        let file_name = read_string(&mut cursor, 256)?;
        let offset = read_u32(&mut cursor)?;
        let length = read_u32(&mut cursor)?;
        let state = SlfEntryState::from_u8(read_u8(&mut cursor)?);
        read_unused(&mut cursor, 3)?;
        let file_time = read_i64(&mut cursor)?;
        read_unused(&mut cursor, 4)?;
        debug_assert_eq!(cursor.position() as usize, NUM_BYTES);

        return Ok(SlfEntry {
            file_name,
            offset,
            length,
            state,
            file_time,
        });
    }

    // Write the archive entry to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<&Self>
    where
        T: Write,
    {
        const NUM_BYTES: usize = size_of::<DIRENTRY>();
        debug_assert_eq!(NUM_BYTES, 280);

        // use local buffer
        let mut buffer: [u8; NUM_BYTES] = [0u8; NUM_BYTES];
        let mut cursor = Cursor::new(&mut buffer[..]);

        // write to buffer
        write_string(&mut cursor, 256, &self.file_name)?;
        write_u32(&mut cursor, self.offset)?;
        write_u32(&mut cursor, self.length)?;
        write_u8(&mut cursor, self.state.into_u8())?;
        write_unused(&mut cursor, 3)?;
        write_i64(&mut cursor, self.file_time)?;
        write_unused(&mut cursor, 4)?;
        debug_assert_eq!(cursor.position() as usize, NUM_BYTES);

        // write data
        output.write_all(&buffer)?;

        return Ok(&self);
    }

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

    // Data starts after the specified offset.
    #[allow(dead_code)]
    pub fn start_of_data(&self) -> SeekFrom {
        return SeekFrom::Start(self.offset as u64);
    }
}

impl SlfEntryState {
    // TODO ::std::convert::Into, ::std::convert::From?

    // Create an archive entry state from a raw state.
    pub fn from_u8(value: u8) -> Self {
        return match value {
            0x00 => SlfEntryState::Ok,
            0xFF => SlfEntryState::Deleted,
            0x01 => SlfEntryState::Old,
            0xFE => SlfEntryState::DoesNotExist,
            _ => SlfEntryState::Unknown(value),
        };
    }

    // Get the raw state of an archive entry state.
    pub fn into_u8(&self) -> u8 {
        return match self {
            SlfEntryState::Ok => 0x00,
            SlfEntryState::Deleted => 0xFF,
            SlfEntryState::Old => 0x01,
            SlfEntryState::DoesNotExist => 0xFE,
            SlfEntryState::Unknown(value) => *value,
        };
    }
}

fn read_unused<T>(input: &mut T, num_bytes: usize) -> Result<()>
where
    T: Read,
{
    let mut buffer = vec![0u8; num_bytes];
    input.read_exact(&mut buffer)?;
    return Ok(());
}

fn write_unused<T>(output: &mut T, num_bytes: usize) -> Result<()>
where
    T: Write,
{
    let mut buffer = vec![0u8; num_bytes];
    output.write_all(&mut buffer)?;
    return Ok(());
}

fn read_u8<T>(input: &mut T) -> Result<u8>
where
    T: Read,
{
    return input.read_u8();
}

fn write_u8<T>(output: &mut T, value: u8) -> Result<()>
where
    T: Write,
{
    output.write_u8(value)?;
    return Ok(());
}

fn read_u16<T>(input: &mut T) -> Result<u16>
where
    T: Read,
{
    return input.read_u16::<LittleEndian>();
}

fn write_u16<T>(output: &mut T, value: u16) -> Result<()>
where
    T: Write,
{
    output.write_u16::<LittleEndian>(value)?;
    return Ok(());
}

fn read_i32<T>(input: &mut T) -> Result<i32>
where
    T: Read,
{
    return input.read_i32::<LittleEndian>();
}

fn write_i32<T>(output: &mut T, value: i32) -> Result<()>
where
    T: Write,
{
    output.write_i32::<LittleEndian>(value)?;
    return Ok(());
}

fn read_u32<T>(input: &mut T) -> Result<u32>
where
    T: Read,
{
    return input.read_u32::<LittleEndian>();
}

fn write_u32<T>(output: &mut T, value: u32) -> Result<()>
where
    T: Write,
{
    output.write_u32::<LittleEndian>(value)?;
    return Ok(());
}

fn read_i64<T>(input: &mut T) -> Result<i64>
where
    T: Read,
{
    return input.read_i64::<LittleEndian>();
}

fn write_i64<T>(output: &mut T, value: i64) -> Result<()>
where
    T: Write,
{
    output.write_i64::<LittleEndian>(value)?;
    return Ok(());
}

fn read_string<T>(input: &mut T, num_bytes: usize) -> Result<String>
where
    T: Read,
{
    let mut buffer = vec![0u8; num_bytes];
    input.read_exact(&mut buffer)?;
    // must be nul terminated and valid utf8
    return match buffer.iter().position(|&byte| byte == 0) {
        Some(position) => match ::std::str::from_utf8(&buffer[..position]) {
            Ok(s) => Ok(s.to_string()),
            Err(e) => Err(Error::new(InvalidData, e)),
        },
        None => Err(Error::new(InvalidData, "string is not nul terminated")),
    };
}

fn write_string<T>(output: &mut T, num_bytes: usize, string: &String) -> Result<()>
where
    T: Write,
{
    let mut buffer = vec![0u8; num_bytes];
    let string_bytes = string.as_bytes();
    if string_bytes.len() >= buffer.len() {
        return Err(Error::new(InvalidData, "string is too long"));
    }
    buffer[..string_bytes.len()].copy_from_slice(&string_bytes);
    output.write_all(&buffer)?;
    return Ok(());
}
