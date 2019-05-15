#pragma once

#include <array>
#include <cstdint>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	enum class ERenderCommand : std::uint32_t
	{
		ClearScreen
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ERenderMode : std::uint8_t
	{
		Normal,
		PostEffect,
		HUD,
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EViewport : std::uint8_t
	{
		Viewport0,
		Viewport1,
		Viewport2,
		Viewport3,
		Viewport4,
		Viewport5,
		Viewport6,
		Viewport7,
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ERenderCommandType : std::uint8_t
	{
		PreDrawCalls,
		NoCustomCommand,
		PostDrawCalls
	};

//////////////////////////////////////////////////////////////////////////

#define GET_RENDER_MASK(num_bits) ((1 << (num_bits)) - 1)
#define SHIFT_RENDER_MASK(value, mask_bits, shift_bits)                       \
	((static_cast<std::uint32_t>(value) & GET_RENDER_MASK(mask_bits))         \
	 << shift_bits)

	struct IOERenderQueueItem
	{
		/**
		 * The ID of the render command. This is used to sort the render queue
		 * to ensure the fewest state changes possible.
		 *
		 *																			+---------------------+
		 *                                                                          |
		 *22                  |
		 *			+-------------+----------+-------+-------------+----------------+-------+-------------+
		 * Bits:	| 2           | 3        | 2     | 1           | 2
		 *| 5     | 17          |
		 *			+-------------+----------+-------+-------------+----------------+-------+-------------+
		 * Meaning	| Render Mode | Viewport | Layer | Translucent | Custom
		 *Command | Depth | Material ID |
		 *			+-------------+----------+-------+-------------+----------------+-------+-------------+
		 *																			| Custom command enum
		 *|
		 *																			+---------------------+
		 *
		 * Render mode: Normal, Post-effect or HUD
		 * Viewport: Viewports 0 - 7
		 * Layer: Individual layer within a viewport (0 - 3)
		 * Translucent: opaque or translucent
		 * Custom Command: Either pre-draw calls (0), an ordinary draw call (1)
		 *or a post-draw call (2).
		 *  If anything other than 1, then depth/material id are treated as a
		 *custom command enum.
		 * Depth: Custom depth sorting for the object
		 * Material ID: A unique ID for a material. 17 bits allows us 131k
		 *unique materials which should
		 *	certainly be plentiful for any future situations. Could always
		 *decrease depth to give us more
		 *  materials if needed.
		 * Custom command enum: If custom command is anything other than 1,
		 *then we treat this as a single
		 *  enum and match it against specific render commands.
		 */

		static const std::uint32_t NUM_BITS_MATERIAL_ID = 17;
		static const std::uint32_t NUM_BITS_DEPTH		= 5;
		static const std::uint32_t NUM_BITS_RENDER_COMMAND =
			NUM_BITS_MATERIAL_ID + NUM_BITS_DEPTH;
		static const std::uint32_t NUM_BITS_CUSTOM_COMMAND = 2;
		static const std::uint32_t NUM_BITS_TRANSLUCENT	= 1;
		static const std::uint32_t NUM_BITS_LAYER		   = 2;
		static const std::uint32_t NUM_BITS_VIEWPORT	   = 3;
		static const std::uint32_t NUM_BITS_RENDER_MODE	= 2;

		static const std::uint32_t BIT_POS_MATERIAL_ID = 0;
		static const std::uint32_t BIT_POS_DEPTH =
			BIT_POS_MATERIAL_ID + NUM_BITS_MATERIAL_ID;
		static const std::uint32_t BIT_POS_RENDER_COMMAND = BIT_POS_DEPTH;
		static const std::uint32_t BIT_POS_CUSTOM_COMMAND =
			BIT_POS_DEPTH + NUM_BITS_DEPTH;
		static const std::uint32_t BIT_POS_TRANSLUCENT =
			BIT_POS_CUSTOM_COMMAND + BIT_POS_CUSTOM_COMMAND;
		static const std::uint32_t BIT_POS_LAYER =
			BIT_POS_TRANSLUCENT + BIT_POS_TRANSLUCENT;
		static const std::uint32_t BIT_POS_VIEWPORT =
			BIT_POS_LAYER + NUM_BITS_LAYER;
		static const std::uint32_t BIT_POS_RENDER_MODE =
			BIT_POS_VIEWPORT + NUM_BITS_VIEWPORT;

		static_assert(BIT_POS_RENDER_MODE + NUM_BITS_RENDER_MODE == 32,
					  "All subsections of command must add up to 32 bits");

		IOERenderQueueItem()
			: m_uCommandID(0)
		{
		}

	public:
		std::uint32_t GetRenderCommandID() const
		{
			return m_uCommandID;
		}

		FORCEINLINE_DEBUGGABLE void
		SetRenderCommandID(ERenderMode eRenderMode, EViewport eViewport,
						   std::uint8_t uViewportLayer, bool bTranslucent,
						   ERenderCommandType eRenderCommandType,
						   ERenderCommand eRenderCommand)
		{
			m_uCommandID = SHIFT_RENDER_MASK(eRenderMode, NUM_BITS_RENDER_MODE,
											 BIT_POS_RENDER_MODE) |
				SHIFT_RENDER_MASK(eViewport, NUM_BITS_VIEWPORT,
								  BIT_POS_VIEWPORT) |
				SHIFT_RENDER_MASK(uViewportLayer, NUM_BITS_LAYER,
								  BIT_POS_LAYER) |
				SHIFT_RENDER_MASK(bTranslucent, NUM_BITS_TRANSLUCENT,
								  BIT_POS_TRANSLUCENT) |
				SHIFT_RENDER_MASK(eRenderCommandType, NUM_BITS_CUSTOM_COMMAND,
								  BIT_POS_CUSTOM_COMMAND) |
				SHIFT_RENDER_MASK(eRenderCommand, NUM_BITS_RENDER_COMMAND,
								  BIT_POS_RENDER_COMMAND);
		}

		FORCEINLINE_DEBUGGABLE void
		SetRenderCommandID(ERenderMode eRenderMode, EViewport eViewport,
						   std::uint8_t uViewportLayer, bool bTranslucent,
						   std::uint8_t uDepth, std::uint32_t uMaterialID)
		{
			m_uCommandID = SHIFT_RENDER_MASK(eRenderMode, NUM_BITS_RENDER_MODE,
											 BIT_POS_RENDER_MODE) |
				SHIFT_RENDER_MASK(eViewport, NUM_BITS_VIEWPORT,
								  BIT_POS_VIEWPORT) |
				SHIFT_RENDER_MASK(uViewportLayer, NUM_BITS_LAYER,
								  BIT_POS_LAYER) |
				SHIFT_RENDER_MASK(bTranslucent, NUM_BITS_TRANSLUCENT,
								  BIT_POS_TRANSLUCENT) |
				SHIFT_RENDER_MASK(uDepth, NUM_BITS_DEPTH, BIT_POS_DEPTH) |
				SHIFT_RENDER_MASK(uMaterialID, NUM_BITS_MATERIAL_ID,
								  BIT_POS_MATERIAL_ID);
		}

	private:
		std::uint32_t m_uCommandID;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOERenderQueue
	{
		static const std::int32_t ms_nRenderQueueCount = 16 * 1024;

	public:
	private:
		std::array<IOERenderQueueItem, ms_nRenderQueueCount> m_arrQueue;

		std::int32_t m_nNumQueue;
	};

	//////////////////////////////////////////////////////////////////////////

	extern IOERenderQueue g_tRenderQueue;

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE