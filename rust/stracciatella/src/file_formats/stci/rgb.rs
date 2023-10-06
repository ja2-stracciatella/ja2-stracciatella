//! This module contains functionality around RGB STCI images

use super::{STCI_RGB565_BLUE_MASK, STCI_RGB565_GREEN_MASK, STCI_RGB565_RED_MASK};
use std::io::{
    Error,
    ErrorKind::{InvalidData, InvalidInput},
    Read, Result, Write,
};

use byteorder::{ReadBytesExt, WriteBytesExt, LE};

/// Mask part of the rgb STCI header
///
/// The mask includes masks for all RGBA channels.
///
/// We only support RGB565 without alpha channel right now. All other masks will result
/// in an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciMaskRgb(pub u32, pub u32, pub u32, pub u32);

impl StciMaskRgb {
    /// Read the mask part from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let red = input.read_u32::<LE>()?;
        let green = input.read_u32::<LE>()?;
        let blue = input.read_u32::<LE>()?;
        let alpha = input.read_u32::<LE>()?;

        if red != STCI_RGB565_RED_MASK
            || green != STCI_RGB565_GREEN_MASK
            || blue != STCI_RGB565_BLUE_MASK && alpha != 0
        {
            return Err(Error::new(InvalidData, "expected RGB565 mask values"));
        }

        Ok(Self(red, green, blue, alpha))
    }

    /// Write the mask part to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        if self.0 != STCI_RGB565_RED_MASK
            || self.1 != STCI_RGB565_GREEN_MASK
            || self.2 != STCI_RGB565_BLUE_MASK && self.3 != 0
        {
            return Err(Error::new(InvalidInput, "expected RGB565 mask values"));
        }

        output.write_u32::<LE>(self.0)?;
        output.write_u32::<LE>(self.1)?;
        output.write_u32::<LE>(self.2)?;
        output.write_u32::<LE>(self.3)?;
        Ok(())
    }
}

impl Default for StciMaskRgb {
    fn default() -> Self {
        Self(
            STCI_RGB565_RED_MASK,
            STCI_RGB565_GREEN_MASK,
            STCI_RGB565_BLUE_MASK,
            0,
        )
    }
}

/// Color depth part of the rgb STCI header
///
/// The mask includes color depths for all RGBA channels.
///
/// We only support RGB565 without alpha channel right now. All other color depths will result
/// in an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StciColorDepthRgb(pub u8, pub u8, pub u8, pub u8);

impl StciColorDepthRgb {
    /// Read the color depth part from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let red = input.read_u8()?;
        let green = input.read_u8()?;
        let blue = input.read_u8()?;
        let alpha = input.read_u8()?;

        if red != 5 || green != 6 || blue != 5 || alpha != 0 {
            return Err(Error::new(
                InvalidData,
                format!(
                    "only supporting RGB5650 color depth in rgb stci images, got RGB{}{}{}{} in header",
                    red, green, blue, alpha
                ),
            ));
        }

        Ok(Self(red, green, blue, alpha))
    }

    /// Write the color depth part to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        if self.0 != 5 || self.1 != 6 || self.2 != 5 || self.3 != 0 {
            return Err(Error::new(
                InvalidInput,
                format!(
                    "only supporting RGB5650 color depth in rgb stci images, got RGB{}{}{}{}",
                    self.0, self.1, self.2, self.3
                ),
            ));
        }

        output.write_u8(self.0)?;
        output.write_u8(self.1)?;
        output.write_u8(self.2)?;
        output.write_u8(self.3)?;
        Ok(())
    }
}

impl Default for StciColorDepthRgb {
    fn default() -> Self {
        Self(5, 6, 5, 0)
    }
}

/// Rgb specific part of the STCI header
#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct StciHeaderRgb {
    pub mask: StciMaskRgb,
    pub color_depth: StciColorDepthRgb,
}

impl StciHeaderRgb {
    /// Read the rgb specific part from input.
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let mask = StciMaskRgb::from_input(input)?;
        let color_depth = StciColorDepthRgb::from_input(input)?;
        Ok(Self { mask, color_depth })
    }

    /// Write the rgb specific part to output.
    #[allow(dead_code)]
    pub fn to_output<T>(&self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        self.mask.to_output(output)?;
        self.color_depth.to_output(output)?;
        Ok(())
    }
}
