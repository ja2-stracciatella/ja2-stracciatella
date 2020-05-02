//! This module contains code to read and write Sir-Tech's Crazy Image (STCI) file format.
//!
//! STCI is a file format that holds information about one or multiple images in Jagged Alliance 2.
//!
//! There are two variants: Indexed and RGB.
//!
//! Indexed STCI images store their colors in a palette and the data as ETRLE compressed indices into
//! this palette. It also can contain multiple sub images, so it can encode e.g. one or multiple images,
//! animations or tiles. Those sub images can have additional metadata associated such as offsets or
//! animation or tile metadata.
//!
//! RGB STCI images store their colors as RGB565 colors in the data section. They cannot have additional
//! metadata and only a single image can be stored in a RGB STCI image.
//!
//! # File Structure
//!
//! Based on `src/sgp/ImgFmt.h`
//!
//! Each file is composed of:
//!
//! - STCI prefix: 4 bytes of value `b"STCI"`
//! - Header (60 bytes)
//! - Palette (only for indexed images, 768 bytes, stored as RGB triplets)
//! - SubImageHeaders (only for indexed images, 16 bytes each)
//! - Data Section
//! - App Data (only for indexed images, even then it is optional, 16 bytes each)
//!
//! # Header Structure
//!
//! - 4 byte unsigned int with the uncompressed size of the included image(s) pixels
//! - 4 byte unsigned int with the compressed size of the included image(s) pixels
//! - 4 byte unsigned int with the value that is transparent in the image. Is always 0 for Jagged Alliance 2 assets.
//! - 4 byte unsigned into with flags for the stci image. Flags determine whether it is the indexed or RGB variant of STCI
//! - 2 byte unsigned int with the height of the image. Ignored for indexed images, as their size is defined in the sub image headers.
//! - 2 byte unsigned int with the width of the image. Ignored for indexed images, as their size is defined in the sub image headers.
//! - 20 bytes with a format specific header. This header part is different for indexed and RGB images
//! - 1 byte unsigned int with color_depth of the image. This is always 8 for indexed and 16 for RGB images for Jagged Alliance 2 assets.
//! - 3 bytes unused. NOTE: This is different from original source code where no unused bytes are defined after the color depth, but
//!   it seems to be required
//! - 4 bytes unsigned int with the size of the app data included in the STCI image. Always 0 for RGB images.
//!
//! # Indexed Format Specific Header Structure
//!
//! - 4 bytes unsigned int with the number of palette colors. Always 256 for Jagged Alliance 2 assets.
//! - 2 bytes unsigned int with the number of sub images in the indexed STCI image
//! - 1 byte unsigned int with the red color depth for the STCI image. Always 8 for Jagged Alliance 2 assets.
//! - 1 byte unsigned int with the green color depth for the STCI image. Always 8 for Jagged Alliance 2 assets.
//! - 1 byte unsigned int with the blue color depth for the STCI image. Always 8 for Jagged Alliance 2 assets.
//! - 11 bytes of unused data
//!
//! # RGB Format Specific Header Structure
//!
//! - 4 bytes unsigned int with the red color mask for the RGB image. Always `0xF800` for Jagged Alliance 2 assets.
//! - 4 bytes unsigned int with the green color mask for the RGB image. Always `0x7E0` for Jagged Alliance 2 assets.
//! - 4 bytes unsigned int with the blue color mask for the RGB image. Always `0x1F` for Jagged Alliance 2 assets.
//! - 4 bytes unsigned int with the alpha color mask for the RGB image. Always `0` for Jagged Alliance 2 assets.
//! - 1 bytes unsigned int with the red color depth for the RGB image. Always `5` for Jagged Alliance 2 assets.
//! - 1 bytes unsigned int with the green color depth for the RGB image. Always `6` for Jagged Alliance 2 assets.
//! - 1 bytes unsigned int with the blue color depth for the RGB image. Always `5` for Jagged Alliance 2 assets.
//! - 1 bytes unsigned int with the alpha color depth for the RGB image. Always `0` for Jagged Alliance 2 assets.
//!
//! # Sub Image Header Stucture
//!
//! - 4 bytes unsigned int with the data offset from data section start
//! - 4 bytes unsigned int with the compressed data length for this sub image
//! - 2 bytes signed int with the x offset for this sub image
//! - 2 bytes signed int with the y offset for this sub image
//! - 2 bytes unsigned int with the height of this sub image
//! - 2 bytes unsigned int with the width of this sub image
//!
//! # Indexed Data Section Structure
//!
//! The data section for indexed images contains all pixel data for each sub image in consecutive order.
//! The data is ETRLE compressed for each sub image. When decompressed it can be used to index into the
//! palette of the STCI image.
//!
//! # RGB Data Section Structure
//!
//! The data section for RGB images contains all pixel data for the RGB image in RGB565 color (which is represented
//! as an 2 byte unsigned int).
//!
//! # App Data Structure
//!
//! App Data is optionally there for indexed STCI images. It contains additional information for the tile and/or
//! animation system. It is only read when the app data size field in the header is > 0.
//!
//! - 1 byte unsigned int for wall orientation
//! - 1 byte unsigned int for number of tiles
//! - 2 byte unsigned int for tile location index
//! - 3 unused bytes
//! - 1 byte unsigned int for current animation keyframe. Seems to be `0` for Jagged Alliance 2 assets.
//! - 1 byte unsigned int for the number of frames for animation
//! - 1 byte unsigned int for flags on the sub image
//! - 6 unused bytes

use super::{StracciatellaReadExt, StracciatellaWriteExt};
use bitflags::bitflags;
use byteorder::{ReadBytesExt, WriteBytesExt, LE};
use std::io::{
    Cursor, Error,
    ErrorKind::{InvalidData, InvalidInput, UnexpectedEof},
    Read, Result, Seek, SeekFrom, Write,
};

mod color;
pub mod etrle;
pub mod indexed;
pub mod rgb;

pub use color::*;
use indexed::*;
use rgb::*;

pub use indexed::{StciAppData, StciPalette, StciSubImage};

/// Representation of the size part of the STCI header.
///
/// - For indexed images:
///   - `original`: Accumulated uncompressed size of the data of all sub images
///   - `stored`: Accumulated compressed size of the data of all sub images
/// - For rgb images:
///   - `original` == `stored`: Uncompressed size of the data in the STCI image
#[derive(Debug, Default, Clone, PartialEq)]
struct StciSize {
    original: u32,
    stored: u32,
}

impl StciSize {
    /// Read the sizes from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let original = input.read_u32::<LE>()?;
        let stored = input.read_u32::<LE>()?;

        Ok(Self { original, stored })
    }

    /// Write the sizes to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_u32::<LE>(self.original)?;
        output.write_u32::<LE>(self.stored)?;
        Ok(())
    }
}

bitflags! {
    /// Flags that can be set on an STCI image
    pub struct StciFlags: u32 {
        /// Sets the STCI to be ETRLE compressed. Needs to be set for indexed STCI images.
        const ETRLE_COMPRESSED = 0x0020;
        /// Sets the STCI to be zlib compressed. Not supported. Setting this flag will result in an error.
        const ZLIB_COMPRESSED = 0x0010;
        /// Sets the STCI to be an indexed STCI.
        const INDEXED = 0x0008;
        /// Sets the STCI to be a rgb STCI.
        const RGB = 0x0004;
        /// Seems to be unused.
        const ALPHA = 0x0002;
        /// Seems to be unused.
        const TRANSPARENT = 0x0001;
    }
}

impl StciFlags {
    /// Returns proper flags for indexed image
    #[allow(dead_code)]
    fn indexed() -> Self {
        let mut flags = StciFlags::empty();
        flags.insert(StciFlags::INDEXED);
        flags.insert(StciFlags::ETRLE_COMPRESSED);
        flags
    }

    /// Returns proper flags for rgb image
    #[allow(dead_code)]
    fn rgb() -> Self {
        let mut flags = StciFlags::empty();
        flags.insert(StciFlags::RGB);
        flags
    }
}

/// Representation of header parts that are common to indexed and rgb STCI.
///
/// In the original data structure there are rgb/indexed specific header parts
/// between `width` and `color_depth`.
#[derive(Debug, Clone, PartialEq)]
struct StciCommonHeader {
    size: StciSize,
    transparent_value: u32,
    flags: StciFlags,
    height: u16,
    width: u16,
    color_depth: u8,
    app_data_size: u32,
}

impl StciCommonHeader {
    /// Returns proper common header for indexed images
    ///
    /// Unknown values (e.g. sizes are set to 0)
    #[allow(dead_code)]
    fn indexed() -> Self {
        StciCommonHeader {
            size: StciSize::default(),
            transparent_value: etrle::INDEXED_ALPHA_VALUE.into(),
            flags: StciFlags::indexed(),
            height: 0,
            width: 0,
            color_depth: 8,
            app_data_size: 0,
        }
    }

    /// Returns proper common header for rgb images
    ///
    /// Unknown values (e.g. sizes are set to 0)
    #[allow(dead_code)]
    fn rgb() -> Self {
        StciCommonHeader {
            size: StciSize::default(),
            transparent_value: 0,
            flags: StciFlags::rgb(),
            height: 0,
            width: 0,
            color_depth: 16,
            app_data_size: 0,
        }
    }
}

/// Complete representation of the STCI header including common and format specific parts
#[derive(Debug, Clone, PartialEq)]
enum StciHeader {
    Indexed {
        header: StciCommonHeader,
        format_specific_header: StciHeaderIndexed,
    },
    Rgb {
        header: StciCommonHeader,
        format_specific_header: StciHeaderRgb,
    },
}

impl StciHeader {
    /// Read the header from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let size = StciSize::from_input(input)?;
        let transparent_value = input.read_u32::<LE>()?;
        let flags = input.read_u32::<LE>()?;
        let flags = StciFlags::from_bits(flags)
            .ok_or_else(|| Error::new(InvalidData, "could not parse StciFlags"))?;
        let height = input.read_u16::<LE>()?;
        let width = input.read_u16::<LE>()?;
        let is_indexed = flags.intersects(StciFlags::INDEXED);
        let is_rgb = flags.intersects(StciFlags::RGB);

        if flags.intersects(StciFlags::ZLIB_COMPRESSED) {
            return Err(Error::new(
                InvalidData,
                "zlib compressed stci not supported",
            ));
        }

        match (is_indexed, is_rgb) {
            (true, false) => {
                let format_specific_header = StciHeaderIndexed::from_input(input)?;
                let color_depth = input.read_u8()?;
                input.read_unused(3)?;
                let app_data_size = input.read_u32::<LE>()?;
                input.read_unused(12)?;

                if color_depth != 8 {
                    return Err(Error::new(
                        InvalidData,
                        "only supporting 8 bit indexed images",
                    ));
                }
                if transparent_value != u32::from(etrle::INDEXED_ALPHA_VALUE) {
                    return Err(Error::new(
                        InvalidData,
                        "only supporting 0 as transparent value",
                    ));
                }
                if !flags.intersects(StciFlags::ETRLE_COMPRESSED) {
                    return Err(Error::new(
                        InvalidData,
                        "only supporting etrle compressed indexed images",
                    ));
                }

                Ok(StciHeader::Indexed {
                    header: StciCommonHeader {
                        size,
                        transparent_value,
                        flags,
                        width,
                        height,
                        color_depth,
                        app_data_size,
                    },
                    format_specific_header,
                })
            }
            (false, true) => {
                let format_specific_header = StciHeaderRgb::from_input(input)?;
                let color_depth = input.read_u8()?;
                input.read_unused(3)?;
                let app_data_size = input.read_u32::<LE>()?;
                input.read_unused(12)?;

                if color_depth != 16 {
                    return Err(Error::new(InvalidData, "only supporting 16 bit RGB images"));
                }
                if flags.intersects(StciFlags::ETRLE_COMPRESSED) {
                    return Err(Error::new(
                        InvalidData,
                        "not supporting etrle encoded rgb images",
                    ));
                }

                Ok(StciHeader::Rgb {
                    header: StciCommonHeader {
                        size,
                        transparent_value,
                        flags,
                        width,
                        height,
                        color_depth,
                        app_data_size,
                    },
                    format_specific_header,
                })
            }
            (true, true) => Err(Error::new(
                InvalidInput,
                "both INDEXED and RGB flags are set",
            )),
            (false, false) => Err(Error::new(
                InvalidInput,
                "neither INDEXED nor RGB flag is set",
            )),
        }
    }

    /// Write the header to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        match self {
            StciHeader::Indexed { header, .. } | StciHeader::Rgb { header, .. } => {
                header.size.to_output(output)?;
                output.write_u32::<LE>(header.transparent_value)?;
                output.write_u32::<LE>(header.flags.bits())?;
                output.write_u16::<LE>(header.height)?;
                output.write_u16::<LE>(header.width)?;
            }
        }
        match self {
            StciHeader::Indexed {
                format_specific_header,
                ..
            } => format_specific_header.to_output(output)?,
            StciHeader::Rgb {
                format_specific_header,
                ..
            } => format_specific_header.to_output(output)?,
        }
        match self {
            StciHeader::Indexed { header, .. } | StciHeader::Rgb { header, .. } => {
                output.write_u8(header.color_depth)?;
                output.write_unused(3)?;
                output.write_u32::<LE>(header.app_data_size)?;
                output.write_unused(12)?;
            }
        }
        Ok(())
    }
}

/// Complete representation of an STCI image
#[derive(Debug, PartialEq)]
pub enum Stci {
    /// Indexed variant
    Indexed {
        palette: Box<StciPalette>,
        sub_images: Vec<StciSubImage>,
    },
    /// Rgb variant
    Rgb {
        width: u16,
        height: u16,
        data: Vec<StciRgb565>,
    },
}

fn decode_sub_image_headers<T>(
    input: &mut T,
    number_of_sub_images: usize,
) -> Result<Vec<StciSubImageHeader>>
where
    T: Read,
{
    let mut subimage_headers = Vec::with_capacity(number_of_sub_images);
    for _ in 0..number_of_sub_images {
        subimage_headers.push(StciSubImageHeader::from_input(input)?);
    }
    Ok(subimage_headers)
}

fn decode_sub_images<T>(
    subimage_headers: Vec<StciSubImageHeader>,
    decode_app_data: bool,
    input: &mut T,
) -> Result<Vec<StciSubImage>>
where
    T: Read,
{
    let number_of_subimages = subimage_headers.len();
    let mut sub_images = Vec::with_capacity(number_of_subimages);
    let mut current_index: u32 = 0;
    for header in subimage_headers.iter() {
        // We expect the images to be stored in the same order as the headers.
        // This seems to be true for all STCI shipped with Jagged Alliance 2
        if header.data_offset != current_index {
            return Err(Error::new(
                InvalidInput,
                "only supporting continoous offsets",
            ));
        }

        let data_length = header.data_length as usize;
        let expected_decompressed_length =
            header.dimensions.0 as usize * header.dimensions.1 as usize;
        let mut data = Vec::with_capacity(expected_decompressed_length);
        let mut output = Cursor::new(&mut data);
        let mut input = input.take(data_length as u64);
        etrle::etrle_decompress(&mut input, &mut output)?;

        // Check whether we decompressed exactly the number of bytes we need
        if data.len() != expected_decompressed_length {
            return Err(Error::new(
                UnexpectedEof,
                format!(
                    "expected to read {} bytes for sub image data, got {}",
                    expected_decompressed_length,
                    data.len(),
                ),
            ));
        }
        sub_images.push(StciSubImage {
            offset: header.offset,
            dimensions: header.dimensions,
            app_data: None,
            data,
        });

        current_index += header.data_length;
    }
    // App data is optional
    if decode_app_data {
        for sub_image in sub_images.iter_mut() {
            let app_data = StciAppData::from_input(input)?;
            sub_image.app_data = Some(app_data);
        }
    }
    Ok(sub_images)
}

impl Stci {
    /// Check if an input is actually an STCI image.
    ///
    /// This is done by reading the first 4 bytes and checking them against `b"STCI"`.
    /// After reading it seeks back 4 bytes, so we are at the same position as before.
    #[allow(dead_code)]
    pub fn peek_is_stci<T>(input: &mut T) -> Result<bool>
    where
        T: Read + Seek,
    {
        let mut tag = [0u8; 4];
        let e = input.read_exact(&mut tag);
        match e {
            Ok(_) => {}
            Err(ref e) if e.kind() == UnexpectedEof => {
                // We cannot read enough bytes for the tag, so its not an STCI file
                return Ok(false);
            }
            Err(e) => {
                // Forward any other error
                return Err(e);
            }
        }
        input.seek(SeekFrom::Current(-4))?;
        Ok(&tag == b"STCI")
    }

    /// Read an STCI image from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let mut tag = [0u8; 4];
        input.read_exact(&mut tag)?;
        if &tag != b"STCI" {
            return Err(Error::new(InvalidInput, "does not seem to be a stci file"));
        }
        let header = StciHeader::from_input(input)?;
        Ok(match header {
            StciHeader::Rgb { header, .. } => {
                let pixels = header.width as usize * header.height as usize;
                let mut data = Vec::with_capacity(pixels);
                for _ in 0..pixels {
                    data.push(StciRgb565::from_input(input)?)
                }
                Stci::Rgb {
                    width: header.width,
                    height: header.height,
                    data,
                }
            }
            StciHeader::Indexed {
                header,
                format_specific_header,
            } => {
                let palette = StciPalette::from_input(
                    input,
                    format_specific_header.number_of_palette_colors as usize,
                )?;
                let sub_image_headers = decode_sub_image_headers(
                    input,
                    format_specific_header.number_of_images as usize,
                )?;
                let sub_images =
                    decode_sub_images(sub_image_headers, header.app_data_size > 0, input)?;
                Stci::Indexed {
                    palette: Box::new(palette),
                    sub_images,
                }
            }
        })
    }

    /// Write a STCI image to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_all(b"STCI")?;
        match self {
            Stci::Rgb {
                width,
                height,
                data,
            } => {
                let byte_size: usize = std::mem::size_of::<u16>();
                let header = StciHeader::Rgb {
                    header: StciCommonHeader {
                        size: StciSize {
                            original: (data.len() * byte_size) as u32,
                            stored: (data.len() * byte_size) as u32,
                        },
                        width: *width,
                        height: *height,
                        ..StciCommonHeader::rgb()
                    },
                    format_specific_header: StciHeaderRgb::default(),
                };
                header.to_output(output)?;

                for color in data {
                    color.to_output(output)?;
                }
            }
            Stci::Indexed {
                palette,
                sub_images,
            } => {
                let number_of_images = sub_images.len();
                let app_data_size: u32 = sub_images
                    .iter()
                    .map(|sub_image| {
                        if sub_image.app_data.is_some() {
                            STCI_APP_DATA_SIZE as u32
                        } else {
                            0
                        }
                    })
                    .sum();
                let compressed_sub_image_bytes: Result<Vec<Vec<u8>>> = sub_images
                    .iter()
                    .map(|sub_image| {
                        let mut data = Vec::<u8>::new();
                        let mut input = sub_image.data.as_slice();
                        let mut output = Cursor::new(&mut data);

                        etrle::etrle_compress(&mut input, &mut output)?;

                        Ok(data)
                    })
                    .collect();
                let compressed_sub_image_bytes = compressed_sub_image_bytes?;
                let original_size: u32 = sub_images
                    .iter()
                    .map(|sub_image| {
                        u32::from(sub_image.dimensions.0) * u32::from(sub_image.dimensions.1)
                    })
                    .sum();
                let stored_size: u32 = compressed_sub_image_bytes
                    .iter()
                    .map(|v| v.len() as u32)
                    .sum();
                let header = StciHeader::Indexed {
                    header: StciCommonHeader {
                        size: StciSize {
                            original: original_size,
                            stored: stored_size,
                        },
                        app_data_size,
                        ..StciCommonHeader::indexed()
                    },
                    format_specific_header: StciHeaderIndexed {
                        number_of_images: number_of_images as u16,
                        ..StciHeaderIndexed::default()
                    },
                };
                header.to_output(output)?;
                palette.to_output(output)?;

                let mut current_offset: u32 = 0;
                for (index, sub_image) in sub_images.iter().enumerate() {
                    let compressed_data = &compressed_sub_image_bytes[index];
                    let compressed_data_len = compressed_data.len() as u32;
                    let sub_image_header = StciSubImageHeader {
                        data_offset: current_offset,
                        data_length: compressed_data_len,
                        dimensions: sub_image.dimensions,
                        offset: sub_image.offset,
                    };

                    sub_image_header.to_output(output)?;

                    current_offset += compressed_data_len;
                }

                for sub_image_bytes in compressed_sub_image_bytes {
                    output.write_all(&sub_image_bytes)?;
                }

                if app_data_size > 0 {
                    for sub_image in sub_images {
                        if let Some(app_data) = &sub_image.app_data {
                            app_data.to_output(output)?;
                        } else {
                            return Err(Error::new(
                                InvalidInput,
                                "either all or no sub images should have app data",
                            ));
                        }
                    }
                }
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn parse_sti_rgb() {
        let mut sti_rgb: &[u8] = b"STCI\x06\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x01\x00\x03\x00\x00\xF8\x00\x00\xe0\x07\x00\x00\x1f\x00\x00\x00\x00\x00\x00\x00\x05\x06\x05\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xF8\x00\xF8\x00\xF8";
        Stci::from_input(&mut sti_rgb).expect("rgb sti should parse");
    }

    #[test]
    fn test_stci_header_indexed() {
        let header = StciHeader::Indexed {
            header: StciCommonHeader::indexed(),
            format_specific_header: StciHeaderIndexed {
                number_of_images: 1,
                ..StciHeaderIndexed::default()
            },
        };
        let expected_size: usize = 60;
        let mut v = vec![0u8; expected_size];
        let mut c = Cursor::new(&mut v);

        // We dont include the STCI tag in our header so it needs to be 60 instead of 64 bytes
        header
            .to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "header should have {} bytes in size when writing, got {}",
            expected_size,
            c.position(),
        );

        c.set_position(0);
        let read_header =
            StciHeader::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "header should have {} bytes in size when reading, got {}",
            expected_size,
            c.position(),
        );
        assert_eq!(read_header, header)
    }

    #[test]
    fn test_stci_header_rgb() {
        let header = StciHeader::Rgb {
            header: StciCommonHeader {
                height: 1,
                width: 3,
                ..StciCommonHeader::rgb()
            },
            format_specific_header: StciHeaderRgb::default(),
        };
        let expected_size: usize = 60;
        let mut v = vec![0u8; expected_size];
        let mut c = Cursor::new(&mut v);

        // We dont include the STCI tag in our header so it needs to be 60 instead of 64 bytes
        header
            .to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "header should have {} bytes in size when writing, got {}",
            expected_size,
            c.position(),
        );

        c.set_position(0);
        let read_header =
            StciHeader::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "header should have {} bytes in size when reading, got {}",
            expected_size,
            c.position(),
        );
        assert_eq!(read_header, header)
    }

    #[test]
    fn test_stci_rgb() {
        let stci = Stci::Rgb {
            height: 1,
            width: 1,
            data: vec![StciRgb565(0)],
        };
        // expected size is 4 byte STCI tag + 60 byte header + 2 byte data
        let expected_size: usize = 66;
        let mut v = vec![0u8; expected_size];
        let mut c = Cursor::new(&mut v);

        stci.to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when writing, got {}",
            expected_size,
            c.position()
        );

        c.set_position(0);
        let read_stci = Stci::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when reading, got {}",
            expected_size,
            c.position(),
        );
        assert_eq!(read_stci, stci)
    }

    #[test]
    fn test_stci_indexed_no_app_data() {
        let stci = Stci::Indexed {
            palette: Box::new(StciPalette::default()),
            sub_images: vec![
                StciSubImage {
                    offset: (0, 0),
                    dimensions: (1, 1),
                    data: vec![0],
                    app_data: None,
                },
                StciSubImage {
                    offset: (1, -1),
                    dimensions: (1, 1),
                    data: vec![0],
                    app_data: None,
                },
            ],
        };
        // expected size is 4 byte STCI tag + 60 byte header + 3 * 256 bytes palette +  2 * 16 byte sub image header + 2 * 1 byte data
        let expected_size: usize = 866;
        let mut v = vec![0u8; expected_size];
        let mut c = Cursor::new(&mut v);

        stci.to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when writing, got {}",
            expected_size,
            c.position()
        );

        c.set_position(0);
        let read_stci = Stci::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when reading, got {}",
            expected_size,
            c.position(),
        );
        assert_eq!(read_stci, stci)
    }

    #[test]
    fn test_stci_indexed_with_app_data() {
        let stci = Stci::Indexed {
            palette: Box::new(StciPalette::default()),
            sub_images: vec![
                StciSubImage {
                    offset: (0, 0),
                    dimensions: (1, 1),
                    data: vec![0],
                    app_data: Some(StciAppData {
                        wall_orientation: 0,
                        number_of_tiles: 0,
                        tile_location_index: 0,
                        current_frame: 0,
                        number_of_frames: 2,
                        flags: StciAppDataFlags::empty(),
                    }),
                },
                StciSubImage {
                    offset: (1, -1),
                    dimensions: (1, 1),
                    data: vec![0],
                    app_data: Some(StciAppData {
                        wall_orientation: 0,
                        number_of_tiles: 0,
                        tile_location_index: 0,
                        current_frame: 0,
                        number_of_frames: 0,
                        flags: StciAppDataFlags::empty(),
                    }),
                },
            ],
        };
        // expected size is
        // 4 byte STCI tag + 60 byte header + 3 * 256 bytes palette +  2 * 16 byte sub image header + 2 * 1 byte data + 2 * 16 byte app data
        let expected_size: usize = 898;
        let mut v = vec![0u8; expected_size];
        let mut c = Cursor::new(&mut v);

        stci.to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when writing, got {}",
            expected_size,
            c.position()
        );

        c.set_position(0);
        let read_stci = Stci::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            expected_size as u64,
            "stci should have {} bytes in size when reading, got {}",
            expected_size,
            c.position(),
        );
        assert_eq!(read_stci, stci)
    }
}
