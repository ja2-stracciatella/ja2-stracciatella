#include "Types.h"
#include <objbase.h>
#include <initguid.h>

#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include <DDraw.h>
	#include "DirectX_Common.h"
	#include <windows.h>
	#include "Debug.h"
	#include <string.h>
#endif

void  DirectXZeroMem ( void* pMemory, int nSize )
{
	memset ( pMemory, 0, nSize );
}


void DirectXAttempt ( INT32 iErrorCode, INT32 nLine, char *szFilename )
{
#ifdef _DEBUG
	if ( iErrorCode != DD_OK )
	{
		FastDebugMsg("DIRECTX COMMON: DirectX Error\n" );
    FastDebugMsg(DirectXErrorDescription(iErrorCode));
	}
#endif
}

char*	DirectXErrorDescription ( INT32 iDXReturn )
{
    switch( iDXReturn )
    {
      case DD_OK
      : return "No error.\0";
      case DDERR_ALREADYINITIALIZED
      : return "The object has already been initialized.";
      case DDERR_BLTFASTCANTCLIP
      : return "A DirectDrawClipper object is attached to a source surface that has passed into a call to the IDirectDrawSurface2::BltFast method.";
      case DDERR_CANNOTATTACHSURFACE
      : return "A surface cannot be attached to another requested surface.";
      case DDERR_CANNOTDETACHSURFACE
      : return "A surface cannot be detached from another requested surface.";
      case DDERR_CANTCREATEDC
      : return "Windows cannot create any more device contexts (DCs).";
      case DDERR_CANTDUPLICATE
      : return "Primary and 3D surfaces, or surfaces that are implicitly created, cannot be duplicated.";
      case DDERR_CANTLOCKSURFACE
      : return "Access to this surface is refused because an attempt was made to lock the primary surface without DCI support.";
      case DDERR_CANTPAGELOCK
      : return "An attempt to page lock a surface failed. Page lock will not work on a display-memory surface or an emulated primary surface.";
      case DDERR_CANTPAGEUNLOCK
      : return "An attempt to page unlock a surface failed. Page unlock will not work on a display-memory surface or an emulated primary surface.";
      case DDERR_CLIPPERISUSINGHWND
      : return "An attempt was made to set a clip list for a DirectDrawClipper object that is already monitoring a window handle.";
      case DDERR_COLORKEYNOTSET
      : return "No source color key is specified for this operation.";
      case DDERR_CURRENTLYNOTAVAIL
      : return "No support is currently available.";
      case DDERR_DCALREADYCREATED
      : return "A device context (DC) has already been returned for this surface. Only one DC can be retrieved for each surface.";
      case DDERR_DIRECTDRAWALREADYCREATED
      : return "A DirectDraw object representing this driver has already been created for this process.";
      case DDERR_EXCEPTION
      : return "An exception was encountered while performing the requested operation.";
      case DDERR_EXCLUSIVEMODEALREADYSET
      : return "An attempt was made to set the cooperative level when it was already set to exclusive.";
      case DDERR_GENERIC
      : return "There is an undefined error condition.";
      case DDERR_HEIGHTALIGN
      : return "The height of the provided rectangle is not a multiple of the required alignment.";
      case DDERR_HWNDALREADYSET
      : return "The DirectDraw cooperative level window handle has already been set. It cannot be reset while the process has surfaces or palettes created.";
      case DDERR_HWNDSUBCLASSED
      : return "DirectDraw is prevented from restoring state because the DirectDraw cooperative level window handle has been subclassed.";
      case DDERR_IMPLICITLYCREATED
      : return "The surface cannot be restored because it is an implicitly created surface.";
      case DDERR_INCOMPATIBLEPRIMARY
      : return "The primary surface creation request does not match with the existing primary surface.";
      case DDERR_INVALIDCAPS
      : return "One or more of the capability bits passed to the callback function are incorrect.";
      case DDERR_INVALIDCLIPLIST
      : return "DirectDraw does not support the provided clip list.";
      case DDERR_INVALIDDIRECTDRAWGUID
      : return "The globally unique identifier (GUID) passed to the DirectDrawCreate function is not a valid DirectDraw driver identifier.";
      case DDERR_INVALIDMODE
      : return "DirectDraw does not support the requested mode.";
      case DDERR_INVALIDOBJECT
      : return "DirectDraw received a pointer that was an invalid DirectDraw object.";
      case DDERR_INVALIDPARAMS
      : return "One or more of the parameters passed to the method are incorrect.";
      case DDERR_INVALIDPIXELFORMAT
      : return "The pixel format was invalid as specified.";
      case DDERR_INVALIDPOSITION
      : return "The position of the overlay on the destination is no longer legal.";
      case DDERR_INVALIDRECT
      : return "The provided rectangle was invalid.";
      case DDERR_INVALIDSURFACETYPE
      : return "The requested operation could not be performed because the surface was of the wrong type.";
      case DDERR_LOCKEDSURFACES
      : return "One or more surfaces are locked, causing the failure of the requested operation.";
      case DDERR_NO3D
      : return "No 3D hardware or emulation is present.";
      case DDERR_NOALPHAHW
      : return "No alpha acceleration hardware is present or available, causing the failure of the requested operation.";
      case DDERR_NOBLTHW
      : return "No blitter hardware is present.";
      case DDERR_NOCLIPLIST
      : return "No clip list is available.";
      case DDERR_NOCLIPPERATTACHED
      : return "No DirectDrawClipper object is attached to the surface object.";
      case DDERR_NOCOLORCONVHW
      : return "The operation cannot be carried out because no color-conversion hardware is present or available.";
      case DDERR_NOCOLORKEY
      : return "The surface does not currently have a color key.";
      case DDERR_NOCOLORKEYHW
      : return "The operation cannot be carried out because there is no hardware support for the destination color key.";
      case DDERR_NOCOOPERATIVELEVELSET
      : return "A create function is called without the IDirectDraw2::SetCooperativeLevel method being called.";
      case DDERR_NODC
      : return "No DC has ever been created for this surface.";
      case DDERR_NODDROPSHW
      : return "No DirectDraw raster operation (ROP) hardware is available.";
      case DDERR_NODIRECTDRAWHW
      : return "Hardware-only DirectDraw object creation is not possible; the driver does not support any hardware.";
      case DDERR_NODIRECTDRAWSUPPORT
      : return "DirectDraw support is not possible with the current display driver.";
      case DDERR_NOEMULATION
      : return "Software emulation is not available.";
      case DDERR_NOEXCLUSIVEMODE
      : return "The operation requires the application to have exclusive mode, but the application does not have exclusive mode.";
      case DDERR_NOFLIPHW
      : return "Flipping visible surfaces is not supported.";
      case DDERR_NOGDI
      : return "No GDI is present.";
      case DDERR_NOHWND
      : return "Clipper notification requires a window handle, or no window handle has been previously set as the cooperative level window handle.";
      case DDERR_NOMIPMAPHW
      : return "The operation cannot be carried out because no mipmap texture mapping hardware is present or available.";
      case DDERR_NOMIRRORHW
      : return "The operation cannot be carried out because no mirroring hardware is present or available.";
      case DDERR_NOOVERLAYDEST
      : return "The IDirectDrawSurface2::GetOverlayPosition method is called on an overlay that the IDirectDrawSurface2::UpdateOverlay method has not been called on to establish a destination.";
      case DDERR_NOOVERLAYHW
      : return "The operation cannot be carried out because no overlay hardware is present or available.";
      case DDERR_NOPALETTEATTACHED
      : return "No palette object is attached to this surface.";
      case DDERR_NOPALETTEHW
      : return "There is no hardware support for 16- or 256-color palettes.";
      case DDERR_NORASTEROPHW
      : return "The operation cannot be carried out because no appropriate raster operation hardware is present or available.";
      case DDERR_NOROTATIONHW
      : return "The operation cannot be carried out because no rotation hardware is present or available.";
      case DDERR_NOSTRETCHHW
      : return "The operation cannot be carried out because there is no hardware support for stretching.";
      case DDERR_NOT4BITCOLOR
      : return "The DirectDrawSurface object is not using a 4-bit color palette and the requested operation requires a 4-bit color palette.";
      case DDERR_NOT4BITCOLORINDEX
      : return "The DirectDrawSurface object is not using a 4-bit color index palette and the requested operation requires a 4-bit color index palette.";
      case DDERR_NOT8BITCOLOR
      : return "The DirectDrawSurface object is not using an 8-bit color palette and the requested operation requires an 8-bit color palette.";
      case DDERR_NOTAOVERLAYSURFACE
      : return "An overlay component is called for a non-overlay surface.";
      case DDERR_NOTEXTUREHW
      : return "The operation cannot be carried out because no texture-mapping hardware is present or available.";
      case DDERR_NOTFLIPPABLE
      : return "An attempt has been made to flip a surface that cannot be flipped.";
      case DDERR_NOTFOUND
      : return "The requested item was not found.";
      case DDERR_NOTINITIALIZED
      : return "An attempt was made to call an interface method of a DirectDraw object created by CoCreateInstance before the object was initialized.";
      case DDERR_NOTLOCKED
      : return "An attempt is made to unlock a surface that was not locked.";
      case DDERR_NOTPAGELOCKED
      : return "An attempt is made to page unlock a surface with no outstanding page locks.";
      case DDERR_NOTPALETTIZED
      : return "The surface being used is not a palette-based surface.";
      case DDERR_NOVSYNCHW
      : return "The operation cannot be carried out because there is no hardware support for vertical blank synchronized operations.";
      case DDERR_NOZBUFFERHW
      : return "The operation to create a z-buffer in display memory or to perform a blit using a z-buffer cannot be carried out because there is no hardware support for z-buffers.";
      case DDERR_NOZOVERLAYHW
      : return "The overlay surfaces cannot be z-layered based on the z-order because the hardware does not support z-ordering of overlays.";
      case DDERR_OUTOFCAPS
      : return "The hardware needed for the requested operation has already been allocated.";
      case DDERR_OUTOFMEMORY
      : return "DirectDraw does not have enough memory to perform the operation.";
      case DDERR_OUTOFVIDEOMEMORY
      : return "DirectDraw does not have enough display memory to perform the operation.";
      case DDERR_OVERLAYCANTCLIP
      : return "The hardware does not support clipped overlays.";
      case DDERR_OVERLAYCOLORKEYONLYONEACTIVE
      : return "An attempt was made to have more than one color key active on an overlay.";
      case DDERR_OVERLAYNOTVISIBLE
      : return "The IDirectDrawSurface2::GetOverlayPosition method is called on a hidden overlay.";
      case DDERR_PALETTEBUSY
      : return "Access to this palette is refused because the palette is locked by another thread.";
      case DDERR_PRIMARYSURFACEALREADYEXISTS
      : return "This process has already created a primary surface.";
      case DDERR_REGIONTOOSMALL
      : return "The region passed to the IDirectDrawClipper::GetClipList method is too small.";
      case DDERR_SURFACEALREADYATTACHED
      : return "An attempt was made to attach a surface to another surface to which it is already attached.";
      case DDERR_SURFACEALREADYDEPENDENT
      : return "An attempt was made to make a surface a dependency of another surface to which it is already dependent.";
      case DDERR_SURFACEBUSY
      : return "Access to the surface is refused because the surface is locked by another thread.";
      case DDERR_SURFACEISOBSCURED
      : return "Access to the surface is refused because the surface is obscured.";
      case DDERR_SURFACELOST
      : return "Access to the surface is refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have the IDirectDrawSurface2::Restore method called on it.";
      case DDERR_SURFACENOTATTACHED
      : return "The requested surface is not attached.";
      case DDERR_TOOBIGHEIGHT
      : return "The height requested by DirectDraw is too large.";
      case DDERR_TOOBIGSIZE
      : return "The size requested by DirectDraw is too large. However, the individual height and width are OK.";
      case DDERR_TOOBIGWIDTH
      : return "The width requested by DirectDraw is too large.";
      case DDERR_UNSUPPORTED
      : return "The operation is not supported.";
      case DDERR_UNSUPPORTEDFORMAT
      : return "The FourCC format requested is not supported by DirectDraw.";
      case DDERR_UNSUPPORTEDMASK
      : return "The bitmask in the pixel format requested is not supported by DirectDraw.";
      case DDERR_UNSUPPORTEDMODE
      : return "The display is currently in an unsupported mode.";
      case DDERR_VERTICALBLANKINPROGRESS
      : return "A vertical blank is in progress.";
      case DDERR_WASSTILLDRAWING
      : return "The previous blit operation that is transferring information to or from this surface is incomplete.";
      case DDERR_WRONGMODE
      : return "This surface cannot be restored because it was created in a different mode.";
      case DDERR_XALIGN
      : return "The provided rectangle was not horizontally aligned on a required boundary.";
      default
      : return "Unrecognized error value.\0";
    }
}
