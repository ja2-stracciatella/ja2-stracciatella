//! This module contains functionality around indexed STCI images

use super::super::{StracciatellaReadExt, StracciatellaWriteExt};
use super::color::StciRgb888;
use byteorder::{LE, ReadBytesExt, WriteBytesExt};
use std::fmt;
use std::io::{
    Error,
    ErrorKind::{InvalidData, InvalidInput},
    Read, Result, Write,
};

/// Represents the color depth defined within the STCI indexed format specific header
///
/// We currently only support RGB888 color depth for palettes, all other color depths in STCI
/// images will result in an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciColorDepthIndexed(pub u8, pub u8, pub u8);

impl StciColorDepthIndexed {
    /// Read the color depth part from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let red = input.read_u8()?;
        let green = input.read_u8()?;
        let blue = input.read_u8()?;

        if red != 8 || green != 8 || blue != 8 {
            return Err(Error::new(
                InvalidData,
                format!(
                    "expected RGB888 palette in indexed STCI, go RGB{}{}{} in header",
                    red, green, blue
                ),
            ));
        }

        Ok(Self(red, green, blue))
    }

    /// Write the color depth part to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        if self.0 != 8 || self.1 != 8 || self.2 != 8 {
            return Err(Error::new(
                InvalidInput,
                format!(
                    "only supporting RGB888 palettes in indexed STCI, got RGB{}{}{}",
                    self.0, self.1, self.2
                ),
            ));
        }
        output.write_u8(self.0)?;
        output.write_u8(self.1)?;
        output.write_u8(self.2)?;
        Ok(())
    }
}

impl Default for StciColorDepthIndexed {
    fn default() -> Self {
        StciColorDepthIndexed(8, 8, 8)
    }
}

/// Represents the indexed format specific part of the STCI header
///
/// We currently only support 256 palette colors, all other numbers of colors
/// will result in an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciHeaderIndexed {
    pub number_of_palette_colors: u32,
    pub number_of_images: u16,
    pub color_depth: StciColorDepthIndexed,
}

impl StciHeaderIndexed {
    /// Read format specific part from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let number_of_palette_colors = input.read_u32::<LE>()?;
        let number_of_images = input.read_u16::<LE>()?;

        if number_of_palette_colors != 256 {
            return Err(Error::new(
                InvalidData,
                format!(
                    "expected 256 palette colors in indexed STCI, got {} in header",
                    number_of_palette_colors,
                ),
            ));
        }
        if number_of_images == 0 {
            return Err(Error::new(
                InvalidData,
                "expected at least one sub image in indexed STCI",
            ));
        }

        let color_depth = StciColorDepthIndexed::from_input(input)?;
        input.read_unused(11)?;

        Ok(Self {
            number_of_palette_colors,
            number_of_images,
            color_depth,
        })
    }

    /// Write format specific part to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        if self.number_of_palette_colors != 256 {
            return Err(Error::new(
                InvalidInput,
                format!(
                    "expected 256 palette colors in indexed STCI, got {}",
                    self.number_of_palette_colors,
                ),
            ));
        }
        if self.number_of_images == 0 {
            return Err(Error::new(
                InvalidData,
                "expected at least one sub image in indexed STCI",
            ));
        }
        output.write_u32::<LE>(self.number_of_palette_colors)?;
        output.write_u16::<LE>(self.number_of_images)?;
        self.color_depth.to_output(output)?;
        output.write_unused(11)?;

        Ok(())
    }
}

impl Default for StciHeaderIndexed {
    fn default() -> Self {
        Self {
            number_of_palette_colors: 256,
            number_of_images: 0,
            color_depth: StciColorDepthIndexed::default(),
        }
    }
}

/// Represents the palette within a STCI image
///
/// The palette of STCI images consists of 256 RGB888 color values.
#[derive(Clone)]
pub struct StciPalette {
    pub colors: [StciRgb888; 256],
}

impl Default for StciPalette {
    fn default() -> Self {
        Self {
            colors: [StciRgb888(0, 0, 0); 256],
        }
    }
}

impl fmt::Debug for StciPalette {
    fn fmt(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        self.colors[..].fmt(formatter)
    }
}

impl PartialEq for StciPalette {
    fn eq(&self, other: &StciPalette) -> bool {
        self.colors[..].eq(&other.colors[..])
    }
}

impl StciPalette {
    /// Read the palette from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T, number_of_colors: usize) -> Result<Self>
    where
        T: Read,
    {
        if number_of_colors != 256 {
            return Err(Error::new(
                InvalidData,
                format!("expected 256 colors for palette, got {}", number_of_colors),
            ));
        }
        let mut colors = [StciRgb888(0, 0, 0); 256];
        for color in colors.iter_mut() {
            *color = StciRgb888::from_input(input)?;
        }
        Ok(Self { colors })
    }

    /// Write the palette to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        for color in &self.colors[..] {
            color.to_output(output)?;
        }
        Ok(())
    }
}

/// Size of a single sub image header in STCI files in bytes
#[allow(dead_code)]
pub const STCI_SUB_IMAGE_HEADER_SIZE: usize = 16;

/// This represents a sub image header within an indexed STCI image
///
/// Sub image headers determine where the data of a specific sub image
/// is located and it's dimensions and offset when rendered.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciSubImageHeader {
    pub data_offset: u32,
    pub data_length: u32,
    pub offset: (i16, i16),
    pub dimensions: (u16, u16),
}

impl StciSubImageHeader {
    /// Read the sub image header from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let data_offset = input.read_u32::<LE>()?;
        let data_length = input.read_u32::<LE>()?;
        let offset_x = input.read_i16::<LE>()?;
        let offset_y = input.read_i16::<LE>()?;
        let dimensions_y = input.read_u16::<LE>()?;
        let dimensions_x = input.read_u16::<LE>()?;

        Ok(Self {
            data_offset,
            data_length,
            offset: (offset_x, offset_y),
            dimensions: (dimensions_x, dimensions_y),
        })
    }

    /// Write the sub image header to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_u32::<LE>(self.data_offset)?;
        output.write_u32::<LE>(self.data_length)?;
        output.write_i16::<LE>(self.offset.0)?;
        output.write_i16::<LE>(self.offset.1)?;
        output.write_u16::<LE>(self.dimensions.1)?;
        output.write_u16::<LE>(self.dimensions.0)?;
        Ok(())
    }
}

bitflags::bitflags! {
    /// Some tile-specific flags within the app data in an indexed STCI image
    pub struct StciAppDataFlags: u8 {
        const FULL_TILE = 0x01;
        const ANIMATED_TILE = 0x02;
        const DYNAMIC_TILE = 0x04;
        const INTERACTIVE_TILE = 0x08;
        const IGNORES_HEIGHT = 0x10;
        const USES_LAND_Z = 0x20;
    }
}

/// Ja2 specific metadata that might exist for each sub image in an indexed STCI image
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciAppData {
    pub wall_orientation: u8,
    pub number_of_tiles: u8,
    pub tile_location_index: u16,
    pub current_frame: u8,
    pub number_of_frames: u8,
    pub flags: StciAppDataFlags,
}

/// Size of a single app data stuct in STCI files in bytes
#[allow(dead_code)]
pub const STCI_APP_DATA_SIZE: usize = 16;

impl StciAppData {
    /// Read the app data from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let wall_orientation = input.read_u8()?;
        let number_of_tiles = input.read_u8()?;
        let tile_location_index = input.read_u16::<LE>()?;
        input.read_unused(3)?;
        let current_frame = input.read_u8()?;
        let number_of_frames = input.read_u8()?;
        let flags = input.read_u8()?;
        let flags = StciAppDataFlags::from_bits(flags)
            .ok_or_else(|| Error::new(InvalidData, "expected valid app data flags"))?;
        input.read_unused(6)?;

        Ok(Self {
            wall_orientation,
            number_of_tiles,
            tile_location_index,
            current_frame,
            number_of_frames,
            flags,
        })
    }

    /// Write the app data to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_u8(self.wall_orientation)?;
        output.write_u8(self.number_of_tiles)?;
        output.write_u16::<LE>(self.tile_location_index)?;
        output.write_unused(3)?;
        output.write_u8(self.current_frame)?;
        output.write_u8(self.number_of_frames)?;
        output.write_u8(self.flags.bits())?;
        output.write_unused(6)?;
        Ok(())
    }
}

/// Final representation of a sub image within a indexed STCI image.
///
/// This already holds decompressed ETRLE data, so no need to further decompress anything.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciSubImage {
    pub offset: (i16, i16),
    pub dimensions: (u16, u16),
    pub app_data: Option<StciAppData>,
    pub data: Vec<u8>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn test_indexed_format_specific_header() {
        let format_specific_header = StciHeaderIndexed {
            number_of_palette_colors: 256,
            number_of_images: 1,
            color_depth: StciColorDepthIndexed(8, 8, 8),
        };
        let mut v = vec![0u8; 16];
        let mut c = Cursor::new(&mut v);

        format_specific_header
            .to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            20,
            "format specific header should have 20 bytes in size when writing"
        );

        c.set_position(0);
        let read_header =
            StciHeaderIndexed::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            20,
            "format specific header should have 20 bytes in size when reading"
        );
        assert_eq!(read_header, format_specific_header)
    }

    #[test]
    fn test_stci_sub_image_header() {
        let header = StciSubImageHeader {
            data_offset: 256,
            data_length: 300,
            offset: (1, -1),
            dimensions: (20, 30),
        };
        let mut v = vec![0u8; STCI_SUB_IMAGE_HEADER_SIZE];
        let mut c = Cursor::new(&mut v);

        header
            .to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            STCI_SUB_IMAGE_HEADER_SIZE as u64,
            "app data should have size {} when writing, got {}",
            STCI_SUB_IMAGE_HEADER_SIZE,
            c.position()
        );

        c.set_position(0);
        let read_header =
            StciSubImageHeader::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            STCI_SUB_IMAGE_HEADER_SIZE as u64,
            "app data should have size {} when reading, got {}",
            STCI_SUB_IMAGE_HEADER_SIZE,
            c.position()
        );
        assert_eq!(read_header, header)
    }

    #[test]
    fn test_stci_app_data() {
        let header = StciAppData {
            wall_orientation: 0,
            number_of_tiles: 0,
            tile_location_index: 0,
            current_frame: 0,
            number_of_frames: 0,
            flags: StciAppDataFlags::empty(),
        };
        let mut v = vec![0u8; STCI_APP_DATA_SIZE];
        let mut c = Cursor::new(&mut v);

        header
            .to_output(&mut c)
            .expect("should be possible to write header");
        assert_eq!(
            c.position(),
            STCI_APP_DATA_SIZE as u64,
            "app data should have size {} when writing, got {}",
            STCI_APP_DATA_SIZE,
            c.position()
        );

        c.set_position(0);
        let read_header =
            StciAppData::from_input(&mut c).expect("should be possible to read header");
        assert_eq!(
            c.position(),
            STCI_APP_DATA_SIZE as u64,
            "app data should have size {} when reading, got {}",
            STCI_APP_DATA_SIZE,
            c.position()
        );
        assert_eq!(read_header, header)
    }
}
