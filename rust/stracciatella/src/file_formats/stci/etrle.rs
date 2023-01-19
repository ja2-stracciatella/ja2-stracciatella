//! This module contains functionality around the ETRLE compression used in indexed STCI images
//!
//! ETRLE compression can compress an arbitrary byte array and is optimized to compress consecutive
//! bytes of the same value. This value is chosen beforehand and called `val` in further description.
//! ETRLE compressed data consists of control bytes and data bytes.
//!
//! A control byte consists of the following bits:
//! - 1 Bit (called `com` in further description) to indicate whether this control byte indicates a compressed (1)
//!   sequence of bytes or a non-compressed (0) sequence of bytes
//! - 7 Bit (called `len` in further description) to indicate the length of the current sequence
//!
//! The algorithm works as follows:
//! - Read next control byte
//!   - If `com == 1`, decompress it by writing `val` `len` times to the output
//!   - If not read the next `len` bytes and copy them to the output
//!   - Goto read next control byte

use byteorder::{ReadBytesExt, WriteBytesExt};
use std::io::{Error, ErrorKind, Read, Result, Write};

/// Predetermined value that is compressed
pub const INDEXED_ALPHA_VALUE: u8 = 0;

/// Bit mask to determine whether the current control byte indicates a compressed or non-compressed sequence of bytes
#[allow(dead_code)]
const IS_COMPRESSED_BIT_MASK: u8 = 0x80;

/// Bit mask to determine the length of the current sequence
#[allow(dead_code)]
const COMPRESSED_SEQUENCE_LENGTH_MASK: u8 = 0x7F;

/// Maximum sequence length
#[allow(dead_code)]
const MAX_SEQUENCE_LENGTH: u8 = 127;

/// This function reads ETRLE compressed data from input and writes the decompressed data to output
///
/// This function reads input until the end.
///
/// ```rust
/// use stracciatella::file_formats::stci::etrle::etrle_decompress;
///
/// let mut input: &[u8] = &[0x85];
/// let mut output: Vec<u8> = vec![];
///
/// etrle_decompress(&mut input, &mut output).unwrap();
/// assert_eq!(output, vec![0, 0, 0, 0, 0])
/// ```
#[allow(dead_code)]
pub fn etrle_decompress<R, W>(input: &mut R, output: &mut W) -> Result<()>
where
    R: Read,
    W: Write,
{
    let next_control_byte = input.read_u8();
    match next_control_byte {
        Ok(next_control_byte) => {
            let is_compressed = ((next_control_byte & IS_COMPRESSED_BIT_MASK) >> 7) == 1;
            let length_of_subsequence = next_control_byte & COMPRESSED_SEQUENCE_LENGTH_MASK;

            for _ in 0..length_of_subsequence {
                if is_compressed {
                    output.write_u8(INDEXED_ALPHA_VALUE)?;
                } else {
                    output.write_u8(input.read_u8()?)?;
                }
            }

            etrle_decompress(input, output)
        }
        Err(ref e) if e.kind() == ErrorKind::UnexpectedEof => Ok(()),
        Err(e) => Err(e),
    }
}

/// Writes a sequence to the output
///
/// A sequence in this case consists of only zero bytes or only non-zero bytes
/// and is shorter or equal 127 bytes
fn write_sequence<W>(sequence: &[u8], output: &mut W) -> Result<()>
where
    W: Write,
{
    let sequence_length = sequence.len();
    if sequence_length == 0 {
        return Ok(());
    }
    if sequence_length > usize::from(MAX_SEQUENCE_LENGTH) {
        return Err(Error::new(
            ErrorKind::InvalidInput,
            format!("expected sequence length <= 127, got {}", sequence.len()),
        ));
    }
    let sequence_length = sequence_length as u8;

    if sequence[0] == INDEXED_ALPHA_VALUE {
        output.write_u8(sequence_length | IS_COMPRESSED_BIT_MASK)?;
    } else {
        output.write_u8(sequence_length)?;
        output.write_all(sequence)?;
    }

    Ok(())
}

/// This function reads data from input and writes the ETRLE compressed data to output
///
/// This function reads input until the end.
///
/// ```rust
/// use stracciatella::file_formats::stci::etrle::etrle_compress;
///
/// let mut input: &[u8] = &[0, 0, 0, 0, 0];
/// let mut output: Vec<u8> = vec![];
///
/// etrle_compress(&mut input, &mut output).unwrap();
/// assert_eq!(output, vec![0x85])
/// ```
#[allow(dead_code)]
pub fn etrle_compress<R, W>(input: &mut R, output: &mut W) -> Result<()>
where
    R: Read,
    W: Write,
{
    let mut current_sequence = vec![];
    loop {
        let next_byte = input.read_u8();

        match next_byte {
            Ok(next_byte) => {
                if current_sequence.len() == usize::from(MAX_SEQUENCE_LENGTH) {
                    write_sequence(&current_sequence, output)?;
                    current_sequence = vec![];
                }
                match (current_sequence.get(0), next_byte) {
                    (None, next_byte) => {
                        current_sequence.push(next_byte);
                    }
                    (Some(&INDEXED_ALPHA_VALUE), INDEXED_ALPHA_VALUE) => {
                        current_sequence.push(INDEXED_ALPHA_VALUE);
                    }
                    (Some(_), INDEXED_ALPHA_VALUE) => {
                        write_sequence(&current_sequence, output)?;
                        current_sequence = vec![INDEXED_ALPHA_VALUE];
                    }
                    (Some(&INDEXED_ALPHA_VALUE), v) => {
                        write_sequence(&current_sequence, output)?;
                        current_sequence = vec![v];
                    }
                    (Some(_), v) => {
                        current_sequence.push(v);
                    }
                }
            }
            Err(ref e) if e.kind() == ErrorKind::UnexpectedEof => {
                write_sequence(&current_sequence, output)?;
                break;
            }
            Err(e) => {
                return Err(e);
            }
        }
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn decode() {
        let mut input: &[u8] = &[];
        let mut output: Vec<u8> = vec![];
        etrle_decompress(&mut input, &mut output).unwrap();
        assert_eq!(output.len(), 0);

        let mut input: &[u8] = &[0x02, 2, 3, 0x03, 4, 5, 6];
        let mut output: Vec<u8> = vec![];
        etrle_decompress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![2, 3, 4, 5, 6]);

        let mut input: &[u8] = &[0x85];
        let mut output: Vec<u8> = vec![];
        etrle_decompress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0, 0, 0, 0, 0]);

        let mut input: &[u8] = &[0x82, 0x02, 2, 3, 0x83, 0x03, 4, 5, 6];
        let mut output: Vec<u8> = vec![];
        etrle_decompress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0, 0, 2, 3, 0, 0, 0, 4, 5, 6]);
    }

    #[test]
    fn encode() {
        let mut input: &[u8] = &[];
        let mut output: Vec<u8> = vec![];
        etrle_compress(&mut input, &mut output).unwrap();
        assert_eq!(output.len(), 0);

        let mut input: &[u8] = &[2, 3, 4, 5, 6];
        let mut output: Vec<u8> = vec![];
        etrle_compress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0x05, 2, 3, 4, 5, 6]);

        let mut input: &[u8] = &[0, 0, 0, 0, 0];
        let mut output: Vec<u8> = vec![];
        etrle_compress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0x85]);

        let mut input: &[u8] = &[0, 0, 2, 3, 0, 0, 0, 4, 5, 6];
        let mut output: Vec<u8> = vec![];
        etrle_compress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0x82, 0x02, 2, 3, 0x83, 0x03, 4, 5, 6]);

        let mut input: &[u8] = &[0; 128];
        let mut output: Vec<u8> = vec![];
        etrle_compress(&mut input, &mut output).unwrap();
        assert_eq!(output, vec![0xFF, 0x81]);
    }
}
