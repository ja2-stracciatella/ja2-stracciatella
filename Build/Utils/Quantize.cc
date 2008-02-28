#include "HImage.h"
#include "Quantize.h"


struct NODE
{
	BOOLEAN bIsLeaf;     // TRUE if node has no children
	UINT    nPixelCount; // Number of pixels represented by this leaf
	UINT    nRedSum;     // Sum of red components
	UINT    nGreenSum;   // Sum of green components
	UINT    nBlueSum;    // Sum of blue components
	NODE*   pChild[8];   // Pointers to child nodes
	NODE*   pNext;       // Pointer to next reducible node
};


class CQuantizer
{
	public:
		CQuantizer(UINT nMaxColors, UINT nColorBits);
		~CQuantizer();
		void ProcessImage(const SGPPaletteEntry* pData, int iWidth, int iHeight);
		UINT GetColorCount();
		void GetColorTable(SGPPaletteEntry* prgb);

	private:
		void   AddColor(NODE** ppNode, BYTE r, BYTE g, BYTE b, UINT level);
		NODE*  CreateNode(UINT nLevel);
		void   ReduceTree();

		NODE* m_pTree;
		UINT  m_nLeafCount;
		NODE* m_pReducibleNodes[9];
		UINT  m_nMaxColors;
		UINT  m_nColorBits;
};


CQuantizer::CQuantizer(const UINT nMaxColors, const UINT nColorBits)
{
	m_pTree      = NULL;
	m_nLeafCount = 0;
	for (UINT i = 0; i <= nColorBits; ++i) m_pReducibleNodes[i] = NULL;
	m_nMaxColors = nMaxColors;
	m_nColorBits = nColorBits;
}


static void DeleteTree(NODE** const ppNode)
{
	NODE* const node = *ppNode;
	for (int i = 0; i < 8; i++)
	{
		if (node->pChild[i] != NULL) DeleteTree(&node->pChild[i]);
	}
	delete node;
	*ppNode = NULL;
}


CQuantizer::~CQuantizer()
{
	if (m_pTree != NULL) DeleteTree(&m_pTree);
}


void CQuantizer::ProcessImage(const SGPPaletteEntry* pData, const int iWidth, const int iHeight)
{
	for (size_t i = iWidth * iHeight; i != 0; --i)
	{
		const BYTE b = pData->peRed;
		const BYTE g = pData->peGreen;
		const BYTE r = pData->peBlue;
		AddColor(&m_pTree, pData->peRed, pData->peGreen, pData->peBlue, 0);
		while (m_nLeafCount > m_nMaxColors) ReduceTree();
	}
}


void CQuantizer::AddColor(NODE** const ppNode, const BYTE r, const BYTE g, const BYTE b, const UINT level)
{
	// If the node doesn't exist, create it.
	if (*ppNode == NULL) *ppNode = CreateNode(level);
	NODE* const node = *ppNode;

	if (node->bIsLeaf)
	{
		// Update color information if it's a leaf node.
		node->nPixelCount++;
		node->nRedSum   += r;
		node->nGreenSum += g;
		node->nBlueSum  += b;
	}
	else
	{
		// Recurse a level deeper if the node is not a leaf.
		const int shift = 7 - level;
		const int index =
			(r >> shift & 1) << 2 |
			(g >> shift & 1) << 1 |
			(b >> shift & 1);
		AddColor(&node->pChild[index], r, g, b, level + 1);
	}
}


NODE* CQuantizer::CreateNode(const UINT level)
{
	NODE* const pNode = new NODE;

	pNode->bIsLeaf = level == m_nColorBits;
	if (pNode->bIsLeaf)
	{
		++m_nLeafCount;
	}
	else
	{
		pNode->pNext = m_pReducibleNodes[level];
		m_pReducibleNodes[level] = pNode;
	}
	return pNode;
}


void CQuantizer::ReduceTree()
{
	NODE** const pReducibleNodes = m_pReducibleNodes;
	int i;
	// Find the deepest level containing at least one reducible node.
	for (i = m_nColorBits - 1; i > 0 && pReducibleNodes[i] == NULL; i--) {}

	// Reduce the node most recently added to the list at level i.
	NODE* const pNode = pReducibleNodes[i];
	pReducibleNodes[i] = pNode->pNext;

	UINT nRedSum   = 0;
	UINT nGreenSum = 0;
	UINT nBlueSum  = 0;
	UINT nChildren = 0;

	for (i = 0; i < 8; ++i)
	{
		const NODE* const child = pNode->pChild[i];
		if (child != NULL)
		{
			nRedSum   += child->nRedSum;
			nGreenSum += child->nGreenSum;
			nBlueSum  += child->nBlueSum;
			pNode->nPixelCount += child->nPixelCount;
			delete child;
			pNode->pChild[i] = NULL;
			++nChildren;
		}
	}

	pNode->bIsLeaf    = TRUE;
	pNode->nRedSum    = nRedSum;
	pNode->nGreenSum  = nGreenSum;
	pNode->nBlueSum   = nBlueSum;
	m_nLeafCount     -= nChildren - 1;
}


static size_t GetPaletteColors(const NODE* const pTree, SGPPaletteEntry* const prgb, size_t index)
{
	if (pTree->bIsLeaf)
	{
		SGPPaletteEntry* const dst = &prgb[index++];
		dst->peRed   = pTree->nRedSum   / pTree->nPixelCount;
		dst->peGreen = pTree->nGreenSum / pTree->nPixelCount;
		dst->peBlue  = pTree->nBlueSum  / pTree->nPixelCount;
		dst->peFlags = 0;
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			const NODE* const child = pTree->pChild[i];
			if (child != NULL) index = GetPaletteColors(child, prgb, index);
		}
	}
	return index;
}


UINT CQuantizer::GetColorCount()
{
	return m_nLeafCount;
}


void CQuantizer::GetColorTable(SGPPaletteEntry* const prgb)
{
	GetPaletteColors(m_pTree, prgb, 0);
}


static void MapPalette(UINT8* const pDest, const SGPPaletteEntry* const pSrc, const INT16 sWidth, const INT16 sHeight, const INT16 sNumColors, const SGPPaletteEntry* pTable)
{
	for (size_t i = sWidth * sHeight; i != 0; --i)
	{
		// OK, FOR EACH PALETTE ENTRY, FIND CLOSEST
		INT32  best        = 0;
		UINT32 lowest_dist = 9999999;
		for (INT32 cnt = 0; cnt < sNumColors; ++cnt)
		{
			const SGPPaletteEntry* const a = &pSrc[i];
			const SGPPaletteEntry* const b = &pTable[cnt];
			const INT32  dr   = a->peRed   - b->peRed;
			const INT32  dg   = a->peGreen - b->peGreen;
			const INT32  db   = a->peBlue  - b->peBlue;
			const UINT32 dist = dr * dr + dg * dg + db * db;
			if (dist < lowest_dist)
			{
				lowest_dist = dist;
				best        = cnt;
			}
		}

		// Now we have the lowest value
		// Set into dest
		pDest[i] = best;
	}
}


void QuantizeImage(UINT8* const pDest, const SGPPaletteEntry* const pSrc, const INT16 sWidth, const INT16 sHeight, SGPPaletteEntry* const pPalette)
{
	// FIRST CREATE PALETTE
	CQuantizer q(255, 6);
	q.ProcessImage(pSrc, sWidth, sHeight);
	const INT16 sNumColors = q.GetColorCount();

	memset(pPalette, 0,  sizeof(*pPalette) * 256);
	q.GetColorTable(pPalette);

	// THEN MAP IMAGE TO PALETTE
	// OK, MAPIT!
	MapPalette(pDest, pSrc, sWidth, sHeight, sNumColors, pPalette);
}
