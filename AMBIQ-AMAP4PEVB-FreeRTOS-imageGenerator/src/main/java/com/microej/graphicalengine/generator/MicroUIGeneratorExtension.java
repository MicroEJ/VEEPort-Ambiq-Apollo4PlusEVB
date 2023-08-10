/**
 *
 */
package com.microej.graphicalengine.generator;

import com.microej.tool.ui.generator.BufferedImageLoader;

/**
 *
 */
public class MicroUIGeneratorExtension extends BufferedImageLoader {

	private static final int ALIGNMENT_PIXELS = 1;

	@Override
	public int getStride(int defaultStride) {
		return (getWidth() + ALIGNMENT_PIXELS - 1) & ~(ALIGNMENT_PIXELS - 1);
	}
}
