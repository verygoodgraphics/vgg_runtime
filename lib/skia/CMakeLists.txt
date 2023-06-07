cmake_minimum_required(VERSION 3.8)
project(skia-custom)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(third_party)
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include/third_party/skcms/
  ${CMAKE_INCLUDE_PATH}
)
set(SKIA_SRCS
# as of skia m78 (see include/core/SkMilestone.h)
# the following cpp files are omitted:
#   src/android/*
#   src/atlastext/*
#   src/c/*
#   src/fonts/*
#   src/opts/*
#   src/pdf/*
#   src/svg/*
#   src/xml/*
#   src/xps/*
#
# cpp files in other directories are included with some
# exceptions commented out as follows:
#
#"src/codec/SkAndroidCodecAdapter.cpp"
#"src/codec/SkAndroidCodec.cpp"
  "src/codec/SkBmpBaseCodec.cpp"
  "src/codec/SkBmpCodec.cpp"
  "src/codec/SkBmpMaskCodec.cpp"
  "src/codec/SkBmpRLECodec.cpp"
  "src/codec/SkBmpStandardCodec.cpp"
  "src/codec/SkCodec.cpp"
  "src/codec/SkCodecImageGenerator.cpp"
  "src/codec/SkColorTable.cpp"
  "src/codec/SkEncodedInfo.cpp"
  #"src/codec/SkGifCodec.cpp" # deleted in m92
#"src/codec/SkHeifCodec.cpp"
  "src/codec/SkIcoCodec.cpp"
  "src/codec/SkJpegCodec.cpp"
  "src/codec/SkJpegDecoderMgr.cpp"
  "src/codec/SkJpegUtility.cpp"
  "src/codec/SkMasks.cpp"
  "src/codec/SkMaskSwizzler.cpp"
  #"src/codec/SkOrientationMarker.cpp" # deleted in m92
  "src/codec/SkPngCodec.cpp"
#"src/codec/SkRawCodec.cpp"
#"src/codec/SkSampledCodec.cpp"
  "src/codec/SkSampler.cpp"
  "src/codec/SkStreamBuffer.cpp"
  "src/codec/SkSwizzler.cpp"
  "src/codec/SkWbmpCodec.cpp"
  "src/codec/SkWebpCodec.cpp"
#"src/codec/SkWuffsCodec.cpp"
  "src/core/SkAAClip.cpp"
  "src/core/SkAlphaRuns.cpp"
  "src/core/SkAnalyticEdge.cpp"
  "src/core/SkAnnotation.cpp"
  "src/core/SkArenaAlloc.cpp"
  "src/core/SkATrace.cpp"
  "src/core/SkAutoPixmapStorage.cpp"
  "src/core/SkBBHFactory.cpp"
  "src/core/SkBigPicture.cpp"
  "src/core/SkBitmapCache.cpp"
  #"src/core/SkBitmapController.cpp" # deleted in m92
  "src/core/SkBitmap.cpp"
  "src/core/SkBitmapDevice.cpp"
  "src/core/SkBitmapProcState.cpp"
  "src/core/SkBitmapProcState_matrixProcs.cpp"
  "src/core/SkBlendMode.cpp"
  "src/core/SkBlitRow_D32.cpp"
  "src/core/SkBlitter_A8.cpp"
  "src/core/SkBlitter_ARGB32.cpp"
  "src/core/SkBlitter.cpp"
  "src/core/SkBlitter_RGB565.cpp"
  "src/core/SkBlitter_Sprite.cpp"
  "src/core/SkBlurMask.cpp"
  "src/core/SkBlurMF.cpp"
  "src/core/SkBuffer.cpp"
  "src/core/SkCachedData.cpp"
  "src/core/SkCanvas.cpp"
  "src/core/SkCanvasPriv.cpp"
  "src/core/SkClipStack.cpp"
  "src/core/SkClipStackDevice.cpp"
  "src/core/SkColor.cpp"
  "src/core/SkColorFilter.cpp"
  "src/core/SkColorFilter_Matrix.cpp"
  "src/core/SkColorSpace.cpp"
  "src/core/SkColorSpaceXformSteps.cpp"
  "src/core/SkContourMeasure.cpp"
  "src/core/SkConvertPixels.cpp"
  "src/core/SkCpu.cpp"
  "src/core/SkCubicClipper.cpp"
  "src/core/SkCubicMap.cpp"
  "src/core/SkData.cpp"
  "src/core/SkDataTable.cpp"
  "src/core/SkDebug.cpp"
  "src/core/SkDeferredDisplayList.cpp"
  "src/core/SkDeferredDisplayListRecorder.cpp"
  "src/core/SkDeque.cpp"
  "src/core/SkDescriptor.cpp"
  "src/core/SkDevice.cpp"
  "src/core/SkDistanceFieldGen.cpp"
  "src/core/SkDocument.cpp"
  "src/core/SkDrawable.cpp"
  "src/core/SkDraw_atlas.cpp"
  "src/core/SkDraw.cpp"
  "src/core/SkDrawLooper.cpp"
  "src/core/SkDrawShadowInfo.cpp"
  "src/core/SkDraw_text.cpp"
  "src/core/SkDraw_vertices.cpp"
  "src/core/SkEdgeBuilder.cpp"
  "src/core/SkEdgeClipper.cpp"
  "src/core/SkEdge.cpp"
  "src/core/SkExecutor.cpp"
  "src/core/SkFlattenable.cpp"
  "src/core/SkFont.cpp"
  "src/core/SkFontDescriptor.cpp"
  #"src/core/SkFontLCDConfig.cpp" # deleted in m92
  "src/core/SkFontMgr.cpp"
  "src/core/SkFontStream.cpp"
  #"src/core/SkForceCPlusPlusLinking.cpp" # deleted in m92
  "src/core/SkGaussFilter.cpp"
  "src/core/SkGeometry.cpp"
  "src/core/SkGlobalInitialization_core.cpp"
  "src/core/SkGlyph.cpp"
  "src/core/SkGlyphRun.cpp"
  "src/core/SkGlyphRunPainter.cpp"
  "src/core/SkGpuBlurUtils.cpp"
  "src/core/SkGraphics.cpp"
  "src/core/SkHalf.cpp"
  "src/core/SkICC.cpp"
  "src/core/SkImageFilterCache.cpp"
  "src/core/SkImageFilter.cpp"
  "src/core/SkImageGenerator.cpp"
  "src/core/SkImageInfo.cpp"
  "src/core/SkLatticeIter.cpp"
  "src/core/SkLineClipper.cpp"
  "src/core/SkLocalMatrixImageFilter.cpp"
  "src/core/SkMallocPixelRef.cpp"
  "src/core/SkMaskBlurFilter.cpp"
  "src/core/SkMaskCache.cpp"
  "src/core/SkMask.cpp"
  "src/core/SkMaskFilter.cpp"
  "src/core/SkMaskGamma.cpp"
  "src/core/SkMath.cpp"
  "src/core/SkMatrix44.cpp"
  "src/core/SkMatrix.cpp"
  "src/core/SkMatrixImageFilter.cpp"
  "src/core/SkMD5.cpp"
  "src/core/SkMiniRecorder.cpp"
  #"src/core/SkMipMap.cpp" # deleted in m92
  "src/core/SkModeColorFilter.cpp"
  #"src/core/SkMultiPictureDraw.cpp" # deleted in m92
  #"src/core/SkNormalFlatSource.cpp" # deleted in m92
  #"src/core/SkNormalMapSource.cpp" # deleted in m92
  #"src/core/SkNormalSource.cpp" # deleted in m92
  "src/core/SkOpts.cpp"
  "src/core/SkOverdrawCanvas.cpp"
  "src/core/SkPaint.cpp"
  "src/core/SkPaintPriv.cpp"
  "src/core/SkPath.cpp"
  "src/core/SkPathEffect.cpp"
  "src/core/SkPathMeasure.cpp"
  "src/core/SkPathRef.cpp"
  "src/core/SkPath_serial.cpp"
  "src/core/SkPicture.cpp"
  "src/core/SkPictureData.cpp"
  "src/core/SkPictureFlat.cpp"
  "src/core/SkPictureImageGenerator.cpp"
  "src/core/SkPicturePlayback.cpp"
  "src/core/SkPictureRecord.cpp"
  "src/core/SkPictureRecorder.cpp"
  "src/core/SkPixelRef.cpp"
  "src/core/SkPixmap.cpp"
  "src/core/SkPoint3.cpp"
  "src/core/SkPoint.cpp"
  "src/core/SkPromiseImageTexture.cpp"
  "src/core/SkPtrRecorder.cpp"
  "src/core/SkQuadClipper.cpp"
  "src/core/SkRasterClip.cpp"
  "src/core/SkRasterPipelineBlitter.cpp"
  "src/core/SkRasterPipeline.cpp"
  "src/core/SkReadBuffer.cpp"
  "src/core/SkRecord.cpp"
  "src/core/SkRecordDraw.cpp"
  "src/core/SkRecordedDrawable.cpp"
  "src/core/SkRecorder.cpp"
  "src/core/SkRecordOpts.cpp"
  "src/core/SkRecords.cpp"
  "src/core/SkRect.cpp"
  "src/core/SkRegion.cpp"
  "src/core/SkRegion_path.cpp"
  "src/core/SkRemoteGlyphCache.cpp"
  "src/core/SkResourceCache.cpp"
  "src/core/SkRRect.cpp"
  "src/core/SkRTree.cpp"
  #"src/core/SkRWBuffer.cpp" # deleted in m92
  "src/core/SkScalar.cpp"
  "src/core/SkScalerContext.cpp"
  "src/core/SkScan_AAAPath.cpp"
  "src/core/SkScan_Antihair.cpp"
  "src/core/SkScan_AntiPath.cpp"
  "src/core/SkScan.cpp"
  "src/core/SkScan_Hairline.cpp"
  "src/core/SkScan_Path.cpp"
  "src/core/SkSemaphore.cpp"
  "src/core/SkSharedMutex.cpp"
  "src/core/SkSpecialImage.cpp"
  "src/core/SkSpecialSurface.cpp"
  "src/core/SkSpinlock.cpp"
  "src/core/SkSpriteBlitter_ARGB32.cpp"
  "src/core/SkSpriteBlitter_RGB565.cpp"
  "src/core/SkStream.cpp"
  "src/core/SkStrikeCache.cpp"
  #"src/core/SkStrike.cpp" # deleted in m92
  "src/core/SkStrikeSpec.cpp"
  "src/core/SkString.cpp"
  "src/core/SkStringUtils.cpp"
  "src/core/SkStroke.cpp"
  "src/core/SkStrokeRec.cpp"
  "src/core/SkStrokerPriv.cpp"
  "src/core/SkSurfaceCharacterization.cpp"
  "src/core/SkSwizzle.cpp"
  "src/core/SkTaskGroup.cpp"
  "src/core/SkTextBlob.cpp"
  "src/core/SkThreadID.cpp"
  "src/core/SkTime.cpp"
  #"src/core/SkTLS.cpp" # deleted in m92
  "src/core/SkTSearch.cpp"
  "src/core/SkTypefaceCache.cpp"
  "src/core/SkTypeface.cpp"
  "src/core/SkTypeface_remote.cpp"
  "src/core/SkUnPreMultiply.cpp"
  "src/core/SkUtilsArm.cpp"
  "src/core/SkUtils.cpp"
  "src/core/SkVertices.cpp"
  "src/core/SkVertState.cpp"
  "src/core/SkVMBlitter.cpp"
  "src/core/SkVM.cpp"
  "src/core/SkWriteBuffer.cpp"
  "src/core/SkWriter32.cpp"
  "src/core/SkXfermode.cpp"
  "src/core/SkXfermodeInterpretation.cpp"
  #"src/core/SkYUVASizeInfo.cpp" # deleted in m92
  "src/core/SkYUVMath.cpp"
  "src/core/SkYUVPlanesCache.cpp"
  #"src/effects/imagefilters/SkAlphaThresholdFilter.cpp" # deleted in m92
  "src/effects/imagefilters/SkArithmeticImageFilter.cpp"
  "src/effects/imagefilters/SkBlurImageFilter.cpp"
  "src/effects/imagefilters/SkColorFilterImageFilter.cpp"
  "src/effects/imagefilters/SkComposeImageFilter.cpp"
  #"src/effects/imagefilters/SkDisplacementMapEffect.cpp" # deleted in m92
  "src/effects/imagefilters/SkDropShadowImageFilter.cpp"
  #"src/effects/imagefilters/SkImageFilters.cpp" # deleted in m92
  #"src/effects/imagefilters/SkImageSource.cpp" # deleted in m92
  "src/effects/imagefilters/SkLightingImageFilter.cpp"
  "src/effects/imagefilters/SkMagnifierImageFilter.cpp"
  "src/effects/imagefilters/SkMatrixConvolutionImageFilter.cpp"
  "src/effects/imagefilters/SkMergeImageFilter.cpp"
  "src/effects/imagefilters/SkMorphologyImageFilter.cpp"
  "src/effects/imagefilters/SkOffsetImageFilter.cpp"
  #"src/effects/imagefilters/SkPaintImageFilter.cpp" # deleted in m92
  "src/effects/imagefilters/SkPictureImageFilter.cpp"
  "src/effects/imagefilters/SkTileImageFilter.cpp"
  #"src/effects/imagefilters/SkXfermodeImageFilter.cpp" # deleted in m92
  "src/effects/Sk1DPathEffect.cpp"
  "src/effects/Sk2DPathEffect.cpp"
  "src/effects/SkColorMatrix.cpp"
  "src/effects/SkColorMatrixFilter.cpp"
  "src/effects/SkCornerPathEffect.cpp"
  "src/effects/SkDashPathEffect.cpp"
  "src/effects/SkDiscretePathEffect.cpp"
  "src/effects/SkEmbossMask.cpp"
  "src/effects/SkEmbossMaskFilter.cpp"
  "src/effects/SkHighContrastFilter.cpp"
  "src/effects/SkLayerDrawLooper.cpp"
  "src/effects/SkLumaColorFilter.cpp"
  "src/effects/SkOpPathEffect.cpp"
  "src/effects/SkOverdrawColorFilter.cpp"
  #"src/effects/SkPackBits.cpp" # deleted in m92
  "src/effects/SkShaderMaskFilter.cpp"
  "src/effects/SkTableColorFilter.cpp"
  "src/effects/SkTableMaskFilter.cpp"
  "src/effects/SkTrimPathEffect.cpp"
  "src/gpu/ccpr/GrCCAtlas.cpp"
  "src/gpu/ccpr/GrCCClipPath.cpp"
  "src/gpu/ccpr/GrCCClipProcessor.cpp"
  #"src/gpu/ccpr/GrCCConicShader.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCCoverageProcessor.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCCubicShader.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCDrawPathsOp.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCFiller.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCFillGeometry.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCPathCache.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCPathProcessor.cpp" # deleted in m92
  "src/gpu/ccpr/GrCCPerFlushResources.cpp"
  #"src/gpu/ccpr/GrCCQuadraticShader.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCStrokeGeometry.cpp" # deleted in m92
  #"src/gpu/ccpr/GrCCStroker.cpp" # deleted in m92
  "src/gpu/ccpr/GrCoverageCountingPathRenderer.cpp"
#"src/gpu/ccpr/GrCoverageCountingPathRenderer_none.cpp"
  #"src/gpu/ccpr/GrGSCoverageProcessor.cpp" # deleted in m92
  #"src/gpu/ccpr/GrOctoBounds.cpp" # deleted in m92
  #"src/gpu/ccpr/GrSampleMaskProcessor.cpp" # deleted in m92
  #"src/gpu/ccpr/GrStencilAtlasOp.cpp" # deleted in m92
  #"src/gpu/ccpr/GrVSCoverageProcessor.cpp" # deleted in m92
#"src/gpu/dawn/GrDawnBuffer.cpp"
#"src/gpu/dawn/GrDawnCaps.cpp"
#"src/gpu/dawn/GrDawnGpu.cpp"
#"src/gpu/dawn/GrDawnOpsRenderPass.cpp"
#"src/gpu/dawn/GrDawnProgramBuilder.cpp"
#"src/gpu/dawn/GrDawnProgramDataManager.cpp"
#"src/gpu/dawn/GrDawnRenderTarget.cpp"
#"src/gpu/dawn/GrDawnRingBuffer.cpp"
#"src/gpu/dawn/GrDawnStencilAttachment.cpp" # deleted in m92
#"src/gpu/dawn/GrDawnTexture.cpp"
#"src/gpu/dawn/GrDawnTextureRenderTarget.cpp"
#"src/gpu/dawn/GrDawnUniformHandler.cpp" # deleted in m92
#"src/gpu/dawn/GrDawnUtil.cpp"
#"src/gpu/dawn/GrDawnVaryingHandler.cpp" # deleted in m92
  "src/gpu/effects/generated/GrAARectEffect.cpp"
  "src/gpu/effects/generated/GrAlphaThresholdFragmentProcessor.cpp"
  "src/gpu/effects/generated/GrBlurredEdgeFragmentProcessor.cpp"
  "src/gpu/effects/generated/GrCircleBlurFragmentProcessor.cpp"
  "src/gpu/effects/generated/GrCircleEffect.cpp"
  "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.cpp"
  #"src/gpu/effects/generated/GrComposeLerpEffect.cpp" # deleted in m92
  #"src/gpu/effects/generated/GrComposeLerpRedEffect.cpp" # deleted in m92
  "src/gpu/effects/generated/GrConfigConversionEffect.cpp"
  #"src/gpu/effects/generated/GrConstColorProcessor.cpp" # deleted in m92
  "src/gpu/effects/generated/GrEllipseEffect.cpp"
  #"src/gpu/effects/generated/GrLumaColorFilterEffect.cpp" # deleted in m92
  "src/gpu/effects/generated/GrMagnifierEffect.cpp"
  #"src/gpu/effects/generated/GrMixerEffect.cpp" # deleted in m92
  "src/gpu/effects/generated/GrOverrideInputFragmentProcessor.cpp"
  #"src/gpu/effects/generated/GrPremulInputFragmentProcessor.cpp" # deleted in m92
  "src/gpu/effects/generated/GrRectBlurEffect.cpp"
  "src/gpu/effects/generated/GrRRectBlurEffect.cpp"
  #"src/gpu/effects/generated/GrSimpleTextureEffect.cpp" # deleted in m92
  "src/gpu/effects/GrBezierEffect.cpp"
  "src/gpu/effects/GrBicubicEffect.cpp"
  "src/gpu/effects/GrBitmapTextGeoProc.cpp"
  "src/gpu/effects/GrConvexPolyEffect.cpp"
  "src/gpu/effects/GrCoverageSetOpXP.cpp"
  "src/gpu/effects/GrCustomXfermode.cpp"
  "src/gpu/effects/GrDisableColorXP.cpp"
  "src/gpu/effects/GrDistanceFieldGeoProc.cpp"
  "src/gpu/effects/GrGaussianConvolutionFragmentProcessor.cpp"
  "src/gpu/effects/GrMatrixConvolutionEffect.cpp"
  "src/gpu/effects/GrOvalEffect.cpp"
  "src/gpu/effects/GrPorterDuffXferProcessor.cpp"
  "src/gpu/effects/GrRRectEffect.cpp"
  "src/gpu/effects/GrShadowGeoProc.cpp"
  "src/gpu/effects/GrSkSLFP.cpp"
  #"src/gpu/effects/GrSRGBEffect.cpp" # deleted in m92
  #"src/gpu/effects/GrTextureDomain.cpp" # deleted in m92
  #"src/gpu/effects/GrXfermodeFragmentProcessor.cpp" # deleted in m92
  "src/gpu/effects/GrYUVtoRGBEffect.cpp"
  "src/gpu/geometry/GrPathUtils.cpp"
  "src/gpu/geometry/GrQuad.cpp"
  "src/gpu/geometry/GrQuadUtils.cpp"
  "src/gpu/geometry/GrShape.cpp"
#"src/gpu/gl/android/GrGLMakeNativeInterface_android.cpp"
  "src/gpu/gl/builders/GrGLProgramBuilder.cpp"
  "src/gpu/gl/builders/GrGLShaderStringBuilder.cpp"
#"src/gpu/gl/egl/GrGLMakeNativeInterface_egl.cpp"
#"src/gpu/gl/glfw/GrGLMakeNativeInterface_glfw.cpp"
#"src/gpu/gl/glx/GrGLMakeNativeInterface_glx.cpp"
  "src/gpu/gl/GrGLAssembleGLESInterfaceAutogen.cpp"
  "src/gpu/gl/GrGLAssembleGLInterfaceAutogen.cpp"
  "src/gpu/gl/GrGLAssembleHelpers.cpp"
  "src/gpu/gl/GrGLAssembleInterface.cpp"
  "src/gpu/gl/GrGLAssembleWebGLInterfaceAutogen.cpp"
  "src/gpu/gl/GrGLBuffer.cpp"
  "src/gpu/gl/GrGLCaps.cpp"
  "src/gpu/gl/GrGLContext.cpp"
  "src/gpu/gl/GrGLExtensions.cpp"
  "src/gpu/gl/GrGLGLSL.cpp"
  "src/gpu/gl/GrGLGpu.cpp"
  "src/gpu/gl/GrGLGpuProgramCache.cpp"
  "src/gpu/gl/GrGLInterfaceAutogen.cpp"
#"src/gpu/gl/GrGLMakeNativeInterface_none.cpp"
  "src/gpu/gl/GrGLOpsRenderPass.cpp"
  #"src/gpu/gl/GrGLPath.cpp" # deleted in m92
  #"src/gpu/gl/GrGLPathRendering.cpp" # deleted in m92
  "src/gpu/gl/GrGLProgram.cpp"
  "src/gpu/gl/GrGLProgramDataManager.cpp"
  "src/gpu/gl/GrGLRenderTarget.cpp"
  "src/gpu/gl/GrGLSemaphore.cpp"
  #"src/gpu/gl/GrGLStencilAttachment.cpp" # deleted in m92
  "src/gpu/gl/GrGLTexture.cpp"
  "src/gpu/gl/GrGLTextureRenderTarget.cpp"
  "src/gpu/gl/GrGLTypesPriv.cpp"
  "src/gpu/gl/GrGLUniformHandler.cpp"
  "src/gpu/gl/GrGLUtil.cpp"
  #"src/gpu/gl/GrGLVaryingHandler.cpp" # deleted in m92
  "src/gpu/gl/GrGLVertexArray.cpp"
#"src/gpu/gl/iOS/GrGLMakeNativeInterface_iOS.cpp"
#"src/gpu/gl/mac/GrGLMakeNativeInterface_mac.cpp"
  "src/gpu/glsl/GrGLSLBlend.cpp"
  "src/gpu/glsl/GrGLSL.cpp"
  "src/gpu/glsl/GrGLSLFragmentProcessor.cpp"
  "src/gpu/glsl/GrGLSLFragmentShaderBuilder.cpp"
  "src/gpu/glsl/GrGLSLGeometryProcessor.cpp"
  #"src/gpu/glsl/GrGLSLPrimitiveProcessor.cpp" # deleted in m92
  "src/gpu/glsl/GrGLSLProgramBuilder.cpp"
  "src/gpu/glsl/GrGLSLProgramDataManager.cpp"
  "src/gpu/glsl/GrGLSLShaderBuilder.cpp"
  #"src/gpu/glsl/GrGLSLUtil.cpp" # deleted in m92
  "src/gpu/glsl/GrGLSLVarying.cpp"
  "src/gpu/glsl/GrGLSLVertexGeoBuilder.cpp"
  "src/gpu/glsl/GrGLSLXferProcessor.cpp"
#"src/gpu/gl/win/GrGLMakeNativeInterface_win.cpp"
  "src/gpu/gradients/generated/GrClampedGradientEffect.cpp"
  "src/gpu/gradients/generated/GrDualIntervalGradientColorizer.cpp"
  "src/gpu/gradients/generated/GrLinearGradientLayout.cpp"
  "src/gpu/gradients/generated/GrRadialGradientLayout.cpp"
  "src/gpu/gradients/generated/GrSingleIntervalGradientColorizer.cpp"
  "src/gpu/gradients/generated/GrSweepGradientLayout.cpp"
  #"src/gpu/gradients/generated/GrTextureGradientColorizer.cpp" # deleted in m92
  "src/gpu/gradients/generated/GrTiledGradientEffect.cpp"
  "src/gpu/gradients/generated/GrTwoPointConicalGradientLayout.cpp"
  "src/gpu/gradients/generated/GrUnrolledBinaryGradientColorizer.cpp"
  "src/gpu/gradients/GrGradientBitmapCache.cpp"
  "src/gpu/gradients/GrGradientShader.cpp"
  "src/gpu/GrAHardwareBufferImageGenerator.cpp"
  "src/gpu/GrAHardwareBufferUtils.cpp"
  "src/gpu/GrAuditTrail.cpp"
  "src/gpu/GrBackendSurface.cpp"
  "src/gpu/GrBackendTextureImageGenerator.cpp"
  #"src/gpu/GrBitmapTextureMaker.cpp" # deleted in m92
  "src/gpu/GrBlurUtils.cpp"
  "src/gpu/GrBufferAllocPool.cpp"
  "src/gpu/GrCaps.cpp"
  "src/gpu/GrClipStackClip.cpp"
  #"src/gpu/GrColorSpaceInfo.cpp" # deleted in m92
  "src/gpu/GrColorSpaceXform.cpp"
  "src/gpu/GrContext_Base.cpp"
  #"src/gpu/GrContext.cpp" # deleted in m92
  #"src/gpu/GrContextPriv.cpp" # deleted in m92
  "src/gpu/GrContextThreadSafeProxy.cpp"
  "src/gpu/GrCopyRenderTask.cpp"
  "src/gpu/GrDataUtils.cpp"
  "src/gpu/GrDDLContext.cpp"
  "src/gpu/GrDefaultGeoProcFactory.cpp"
  "src/gpu/GrDistanceFieldGenFromVector.cpp"
  "src/gpu/GrDrawingManager.cpp"
  "src/gpu/GrDrawOpAtlas.cpp"
  "src/gpu/GrDrawOpTest.cpp"
  "src/gpu/GrDriverBugWorkarounds.cpp"
  "src/gpu/GrFixedClip.cpp"
  "src/gpu/GrFragmentProcessor.cpp"
  "src/gpu/GrGpuBuffer.cpp"
  "src/gpu/GrGpu.cpp"
  "src/gpu/GrGpuResource.cpp"
  "src/gpu/GrImageContext.cpp"
  #"src/gpu/GrImageTextureMaker.cpp" # deleted in m92
  #"src/gpu/GrLegacyDirectContext.cpp" # deleted in m92
  "src/gpu/GrMemoryPool.cpp"
  "src/gpu/GrOnFlushResourceProvider.cpp"
  "src/gpu/GrOpFlushState.cpp"
  "src/gpu/GrOpsRenderPass.cpp"
  "src/gpu/GrOpsTask.cpp"
  "src/gpu/GrPaint.cpp"
  #"src/gpu/GrPath.cpp" # deleted in m92
  #"src/gpu/GrPathProcessor.cpp" # deleted in m92
  "src/gpu/GrPathRendererChain.cpp"
  "src/gpu/GrPathRenderer.cpp"
  #"src/gpu/GrPathRendering.cpp" # deleted in m92
#"src/gpu/GrPathRendering_none.cpp" # deleted in m92
  "src/gpu/GrPipeline.cpp"
  #"src/gpu/GrPrimitiveProcessor.cpp" # deleted in m92
  "src/gpu/GrProcessorAnalysis.cpp"
  "src/gpu/GrProcessor.cpp"
  "src/gpu/GrProcessorSet.cpp"
  "src/gpu/GrProcessorUnitTest.cpp"
  "src/gpu/GrProgramDesc.cpp"
  "src/gpu/GrProxyProvider.cpp"
  "src/gpu/GrRecordingContext.cpp"
  #"src/gpu/GrRectanizer_pow2.cpp" # deleted in m92
  #"src/gpu/GrRectanizer_skyline.cpp" # deleted in m92
  "src/gpu/GrReducedClip.cpp"
  #"src/gpu/GrRenderTargetContext.cpp" # deleted in m92
  "src/gpu/GrRenderTarget.cpp"
  "src/gpu/GrRenderTargetProxy.cpp"
  "src/gpu/GrRenderTask.cpp"
  "src/gpu/GrResourceAllocator.cpp"
  "src/gpu/GrResourceCache.cpp"
  "src/gpu/GrResourceProvider.cpp"
  #"src/gpu/GrSamplePatternDictionary.cpp" # deleted in m92
  "src/gpu/GrShaderCaps.cpp"
  "src/gpu/GrShaderUtils.cpp"
  "src/gpu/GrShaderVar.cpp"
  "src/gpu/GrSoftwarePathRenderer.cpp"
  #"src/gpu/GrStencilAttachment.cpp" # deleted in m92
  "src/gpu/GrStencilSettings.cpp"
  "src/gpu/GrStyle.cpp"
  "src/gpu/GrSurfaceContext.cpp"
  "src/gpu/GrSurface.cpp"
  "src/gpu/GrSurfaceProxy.cpp"
  "src/gpu/GrSwizzle.cpp"
  "src/gpu/GrSWMaskHelper.cpp"
  #"src/gpu/GrTessellator.cpp" # deleted in m92
  "src/gpu/GrTestUtils.cpp"
  #"src/gpu/GrTextureAdjuster.cpp" # deleted in m92
  #"src/gpu/GrTextureContext.cpp" # deleted in m92
  "src/gpu/GrTexture.cpp"
  #"src/gpu/GrTextureMaker.cpp" # deleted in m92
  #"src/gpu/GrTextureProducer.cpp" # deleted in m92
  "src/gpu/GrTextureProxy.cpp"
  "src/gpu/GrTextureRenderTargetProxy.cpp"
  "src/gpu/GrTextureResolveRenderTask.cpp"
  "src/gpu/GrTransferFromRenderTask.cpp"
  "src/gpu/GrXferProcessor.cpp"
  #"src/gpu/GrYUVProvider.cpp" # deleted in m92
  "src/gpu/mock/GrMockCaps.cpp"
  "src/gpu/mock/GrMockGpu.cpp"
  "src/gpu/mock/GrMockTypes.cpp"
  "src/gpu/ops/GrAAConvexPathRenderer.cpp"
  "src/gpu/ops/GrAAConvexTessellator.cpp"
  "src/gpu/ops/GrAAHairLinePathRenderer.cpp"
  "src/gpu/ops/GrAALinearizingConvexPathRenderer.cpp"
  "src/gpu/ops/GrAtlasTextOp.cpp"
  "src/gpu/ops/GrClearOp.cpp"
  #"src/gpu/ops/GrClearStencilClipOp.cpp" # deleted in m92
  "src/gpu/ops/GrDashLinePathRenderer.cpp"
  "src/gpu/ops/GrDashOp.cpp"
  #"src/gpu/ops/GrDebugMarkerOp.cpp" # deleted in m92
  "src/gpu/ops/GrDefaultPathRenderer.cpp"
  "src/gpu/ops/GrDrawableOp.cpp"
  "src/gpu/ops/GrDrawAtlasOp.cpp"
  #"src/gpu/ops/GrDrawPathOp.cpp" # deleted in m92
  "src/gpu/ops/GrDrawVerticesOp.cpp"
  "src/gpu/ops/GrFillRectOp.cpp"
  "src/gpu/ops/GrFillRRectOp.cpp"
  "src/gpu/ops/GrLatticeOp.cpp"
  "src/gpu/ops/GrMeshDrawOp.cpp"
  "src/gpu/ops/GrOp.cpp"
  "src/gpu/ops/GrOvalOpFactory.cpp"
  "src/gpu/ops/GrQuadPerEdgeAA.cpp"
  "src/gpu/ops/GrRegionOp.cpp"
  #"src/gpu/ops/GrSemaphoreOp.cpp" # deleted in m92
  "src/gpu/ops/GrShadowRRectOp.cpp"
  "src/gpu/ops/GrSimpleMeshDrawOpHelper.cpp"
  "src/gpu/ops/GrSmallPathRenderer.cpp"
  #"src/gpu/ops/GrStencilAndCoverPathRenderer.cpp" # deleted in m92
  #"src/gpu/ops/GrStencilPathOp.cpp" # deleted in m92
  "src/gpu/ops/GrStrokeRectOp.cpp"
  #"src/gpu/ops/GrTessellatingPathRenderer.cpp" # deleted in m92
  "src/gpu/ops/GrTextureOp.cpp"
  "src/gpu/SkGpuDevice.cpp"
  "src/gpu/SkGpuDevice_drawTexture.cpp"
  "src/gpu/SkGr.cpp"
  "src/gpu/text/GrAtlasManager.cpp"
  "src/gpu/text/GrDistanceFieldAdjustTable.cpp"
  "src/gpu/text/GrSDFMaskFilter.cpp"
  "src/gpu/text/GrStrikeCache.cpp"
  "src/gpu/text/GrTextBlobCache.cpp"
  "src/gpu/text/GrTextBlob.cpp"
  #"src/gpu/text/GrTextBlobVertexRegenerator.cpp" # deleted in m92
  #"src/gpu/text/GrTextContext.cpp" # deleted in m92
#"src/gpu/vk/GrVkAMDMemoryAllocator.cpp"
#"src/gpu/vk/GrVkBuffer.cpp"
#"src/gpu/vk/GrVkBufferView.cpp" # deleted in m92
#"src/gpu/vk/GrVkCaps.cpp"
#"src/gpu/vk/GrVkCommandBuffer.cpp"
#"src/gpu/vk/GrVkCommandPool.cpp"
#"src/gpu/vk/GrVkDescriptorPool.cpp"
#"src/gpu/vk/GrVkDescriptorSet.cpp"
#"src/gpu/vk/GrVkDescriptorSetManager.cpp"
#"src/gpu/vk/GrVkExtensions.cpp"
#"src/gpu/vk/GrVkFramebuffer.cpp"
#"src/gpu/vk/GrVkGpu.cpp"
#"src/gpu/vk/GrVkImage.cpp"
#"src/gpu/vk/GrVkImageView.cpp"
#"src/gpu/vk/GrVkIndexBuffer.cpp" # deleted in m92
#"src/gpu/vk/GrVkInterface.cpp"
#"src/gpu/vk/GrVkMemory.cpp"
#"src/gpu/vk/GrVkOpsRenderPass.cpp"
#"src/gpu/vk/GrVkPipeline.cpp"
#"src/gpu/vk/GrVkPipelineStateBuilder.cpp"
#"src/gpu/vk/GrVkPipelineStateCache.cpp"
#"src/gpu/vk/GrVkPipelineState.cpp"
#"src/gpu/vk/GrVkPipelineStateDataManager.cpp"
#"src/gpu/vk/GrVkRenderPass.cpp"
#"src/gpu/vk/GrVkRenderTarget.cpp"
#"src/gpu/vk/GrVkResourceProvider.cpp"
#"src/gpu/vk/GrVkSampler.cpp"
#"src/gpu/vk/GrVkSamplerYcbcrConversion.cpp"
#"src/gpu/vk/GrVkSecondaryCBDrawContext.cpp"
#"src/gpu/vk/GrVkSemaphore.cpp"
#"src/gpu/vk/GrVkStencilAttachment.cpp" # deleted in m92
#"src/gpu/vk/GrVkTexture.cpp"
#"src/gpu/vk/GrVkTextureRenderTarget.cpp"
#"src/gpu/vk/GrVkTransferBuffer.cpp" # deleted in m92
#"src/gpu/vk/GrVkTypesPriv.cpp"
#"src/gpu/vk/GrVkUniformBuffer.cpp" # deleted in m92
#"src/gpu/vk/GrVkUniformHandler.cpp"
#"src/gpu/vk/GrVkUtil.cpp"
#"src/gpu/vk/GrVkVaryingHandler.cpp"
#"src/gpu/vk/GrVkVertexBuffer.cpp" # deleted in m92
  "src/image/SkImage.cpp"
  "src/image/SkImage_GpuBase.cpp"
  "src/image/SkImage_Gpu.cpp"
  "src/image/SkImage_GpuYUVA.cpp"
  "src/image/SkImage_Lazy.cpp"
  "src/image/SkImage_Raster.cpp"
  "src/image/SkSurface.cpp"
  "src/image/SkSurface_Gpu.cpp"
  "src/image/SkSurface_Raster.cpp"
  "src/images/SkImageEncoder.cpp"
  "src/images/SkJpegEncoder.cpp"
  "src/images/SkJPEGWriteUtility.cpp"
  "src/images/SkPngEncoder.cpp"
  "src/images/SkWebpEncoder.cpp"
  "src/lazy/SkDiscardableMemoryPool.cpp"
  "src/pathops/SkAddIntersections.cpp"
  "src/pathops/SkDConicLineIntersection.cpp"
  "src/pathops/SkDCubicLineIntersection.cpp"
  "src/pathops/SkDCubicToQuads.cpp"
  "src/pathops/SkDLineIntersection.cpp"
  "src/pathops/SkDQuadLineIntersection.cpp"
  "src/pathops/SkIntersections.cpp"
  "src/pathops/SkOpAngle.cpp"
  "src/pathops/SkOpBuilder.cpp"
  "src/pathops/SkOpCoincidence.cpp"
  "src/pathops/SkOpContour.cpp"
  "src/pathops/SkOpCubicHull.cpp"
  "src/pathops/SkOpEdgeBuilder.cpp"
  "src/pathops/SkOpSegment.cpp"
  "src/pathops/SkOpSpan.cpp"
  "src/pathops/SkPathOpsAsWinding.cpp"
  "src/pathops/SkPathOpsCommon.cpp"
  "src/pathops/SkPathOpsConic.cpp"
  "src/pathops/SkPathOpsCubic.cpp"
  "src/pathops/SkPathOpsCurve.cpp"
  "src/pathops/SkPathOpsDebug.cpp"
  "src/pathops/SkPathOpsLine.cpp"
  "src/pathops/SkPathOpsOp.cpp"
  "src/pathops/SkPathOpsQuad.cpp"
  "src/pathops/SkPathOpsRect.cpp"
  "src/pathops/SkPathOpsSimplify.cpp"
  "src/pathops/SkPathOpsTightBounds.cpp"
  "src/pathops/SkPathOpsTSect.cpp"
  "src/pathops/SkPathOpsTypes.cpp"
  "src/pathops/SkPathOpsWinding.cpp"
  "src/pathops/SkPathWriter.cpp"
  "src/pathops/SkReduceOrder.cpp"
  "src/sfnt/SkOTTable_name.cpp"
  "src/sfnt/SkOTUtils.cpp"
  "src/shaders/gradients/Sk4fGradientBase.cpp"
  "src/shaders/gradients/Sk4fLinearGradient.cpp"
  "src/shaders/gradients/SkGradientShader.cpp"
  "src/shaders/gradients/SkLinearGradient.cpp"
  "src/shaders/gradients/SkRadialGradient.cpp"
  "src/shaders/gradients/SkSweepGradient.cpp"
  "src/shaders/gradients/SkTwoPointConicalGradient.cpp"
  "src/shaders/SkBitmapProcShader.cpp"
  "src/shaders/SkColorFilterShader.cpp"
  "src/shaders/SkColorShader.cpp"
  "src/shaders/SkComposeShader.cpp"
  "src/shaders/SkImageShader.cpp"
  #"src/shaders/SkLightingShader.cpp" # deleted in m92
  #"src/shaders/SkLights.cpp" # deleted in m92
  "src/shaders/SkLocalMatrixShader.cpp"
  "src/shaders/SkPerlinNoiseShader.cpp"
  "src/shaders/SkPictureShader.cpp"
  #"src/shaders/SkRTShader.cpp" # deleted in m92
  "src/shaders/SkShader.cpp"
  "src/sksl/ir/SkSLSetting.cpp"
  "src/sksl/ir/SkSLSymbolTable.cpp"
  "src/sksl/ir/SkSLType.cpp"
  "src/sksl/ir/SkSLVariableReference.cpp"
#"src/sksl/lex/Main.cpp"
#"src/sksl/lex/NFA.cpp"
#"src/sksl/lex/RegexNode.cpp"
#"src/sksl/lex/RegexParser.cpp"
  "src/sksl/SkSLASTNode.cpp"
  #"src/sksl/SkSLByteCode.cpp" # deleted in m92
  #"src/sksl/SkSLByteCodeGenerator.cpp" # deleted in m92
  #"src/sksl/SkSLCFGGenerator.cpp" # deleted in m92
  "src/sksl/SkSLCompiler.cpp"
  #"src/sksl/SkSLCPPCodeGenerator.cpp" # deleted in m92
  "src/sksl/SkSLCPPUniformCTypes.cpp"
  #"src/sksl/SkSLGLSLCodeGenerator.cpp" # deleted in m92
  #"src/sksl/SkSLHCodeGenerator.cpp" # deleted in m92
  "src/sksl/SkSLIRGenerator.cpp"
  #"src/sksl/SkSLJIT.cpp" # deleted in m92
  "src/sksl/SkSLLexer.cpp"
#"src/sksl/SkSLMain.cpp"
  #"src/sksl/SkSLMetalCodeGenerator.cpp" # deleted in m92
  "src/sksl/SkSLOutputStream.cpp"
  "src/sksl/SkSLParser.cpp"
  #"src/sksl/SkSLPipelineStageCodeGenerator.cpp" # deleted in m92
  "src/sksl/SkSLSectionAndParameterHelper.cpp"
  #"src/sksl/SkSLSPIRVCodeGenerator.cpp" # deleted in m92
  "src/sksl/SkSLString.cpp"
  "src/sksl/SkSLUtil.cpp"
#"src/utils/mac/SkCreateCGImageRef.cpp"
#"src/utils/mac/SkStream_mac.cpp" # deleted in m92
  #"src/utils/Sk3D.cpp" # deleted in m92
#"src/utils/SkAnimCodecPlayer.cpp"
  "src/utils/SkBase64.cpp"
  "src/utils/SkCamera.cpp"
  "src/utils/SkCanvasStack.cpp"
  "src/utils/SkCanvasStateUtils.cpp"
  "src/utils/SkCharToGlyphCache.cpp"
  "src/utils/SkDashPath.cpp"
  "src/utils/SkEventTracer.cpp"
  "src/utils/SkFloatToDecimal.cpp"
  #"src/utils/SkFrontBufferedStream.cpp" # deleted in m92
  #"src/utils/SkInterpolator.cpp" # deleted in m92
  "src/utils/SkJSON.cpp"
  "src/utils/SkJSONWriter.cpp"
#"src/utils/SkLuaCanvas.cpp" # deleted in m92
#"src/utils/SkLua.cpp" # deleted in m92
  "src/utils/SkMatrix22.cpp"
  "src/utils/SkMultiPictureDocument.cpp"
  "src/utils/SkNullCanvas.cpp"
  "src/utils/SkNWayCanvas.cpp"
  "src/utils/SkOSPath.cpp"
  "src/utils/SkPaintFilterCanvas.cpp"
  "src/utils/SkParseColor.cpp"
  "src/utils/SkParse.cpp"
  "src/utils/SkParsePath.cpp"
  "src/utils/SkPatchUtils.cpp"
  "src/utils/SkPolyUtils.cpp"
  "src/utils/SkShadowTessellator.cpp"
  "src/utils/SkShadowUtils.cpp"
#"src/utils/SkShaperJSONWriter.cpp"
  "src/utils/SkTextUtils.cpp"
  "src/utils/SkThreadUtils_pthread.cpp"
  "src/utils/SkThreadUtils_win.cpp"
  "src/utils/SkUTF.cpp"
  #"src/utils/SkWhitelistTypefaces.cpp" # deleted in m92
#"src/utils/win/SkAutoCoInitialize.cpp"
#"src/utils/win/SkDWrite.cpp"
#"src/utils/win/SkDWriteFontFileStream.cpp"
#"src/utils/win/SkDWriteGeometrySink.cpp"
#"src/utils/win/SkHRESULT.cpp"
#"src/utils/win/SkIStream.cpp"
#"src/utils/win/SkWGL_win.cpp"
  #"third_party/gif/SkGifImageReader.cpp" # deleted in m92
  "third_party/skcms/skcms.cc"
#
# and the following cpp files are platform-specific:
#
#"src/ports/SkDebug_android.cpp"
  "src/ports/SkDebug_stdio.cpp"
#"src/ports/SkDebug_win.cpp"
#"src/ports/SkDiscardableMemory_none.cpp"
#"src/ports/SkFontConfigInterface.cpp"
#"src/ports/SkFontConfigInterface_direct.cpp"
#"src/ports/SkFontConfigInterface_direct_factory.cpp"
  "src/ports/SkFontHost_FreeType_common.cpp"
  "src/ports/SkFontHost_FreeType.cpp"
#"src/ports/SkFontHost_mac.cpp" # deleted in m92
#"src/ports/SkFontHost_win.cpp"
#"src/ports/SkFontMgr_android.cpp"
#"src/ports/SkFontMgr_android_factory.cpp"
#"src/ports/SkFontMgr_android_parser.cpp"
  "src/ports/SkFontMgr_custom.cpp"
  "src/ports/SkFontMgr_custom_directory.cpp"
  "src/ports/SkFontMgr_custom_directory_factory.cpp"
  "src/ports/SkFontMgr_custom_embedded.cpp"
#"src/ports/SkFontMgr_custom_embedded_factory.cpp"
  "src/ports/SkFontMgr_custom_empty.cpp"
#"src/ports/SkFontMgr_custom_empty_factory.cpp"
#"src/ports/SkFontMgr_empty_factory.cpp"
#"src/ports/SkFontMgr_fontconfig.cpp"
#"src/ports/SkFontMgr_fontconfig_factory.cpp"
#"src/ports/SkFontMgr_FontConfigInterface.cpp"
#"src/ports/SkFontMgr_FontConfigInterface_factory.cpp"
#"src/ports/SkFontMgr_fuchsia.cpp"
#"src/ports/SkFontMgr_win_dw.cpp"
#"src/ports/SkFontMgr_win_dw_factory.cpp"
  "src/ports/SkGlobalInitialization_default.cpp"
#"src/ports/SkImageEncoder_CG.cpp"
#"src/ports/SkImageEncoder_WIC.cpp"
#"src/ports/SkImageGeneratorCG.cpp"
#"src/ports/SkImageGenerator_none.cpp"
  "src/ports/SkImageGenerator_skia.cpp"
#"src/ports/SkImageGeneratorWIC.cpp"
  "src/ports/SkMemory_malloc.cpp"
#"src/ports/SkMemory_mozalloc.cpp"
  "src/ports/SkOSFile_posix.cpp"
  "src/ports/SkOSFile_stdio.cpp"
#"src/ports/SkOSFile_win.cpp"
  "src/ports/SkOSLibrary_posix.cpp"
#"src/ports/SkOSLibrary_win.cpp"
#"src/ports/SkRemotableFontMgr_win_dw.cpp"
#"src/ports/SkScalerContext_win_dw.cpp"
#"src/ports/SkTLS_none.cpp" # deleted in m92
  "src/ports/SkTLS_pthread.cpp"
#"src/ports/SkTLS_win.cpp"
#"src/ports/SkTypeface_win_dw.cpp"
)
set(SKIA_SRCS ${SKIA_SRCS}
# the following cpp files are added from m78 to m92:
#
  "src/codec/SkParseEncodedOrigin.cpp"
  "src/core/SkCompressedDataUtils.cpp"
  "src/core/SkFont_serial.cpp"
  "src/core/SkGlyphBuffer.cpp"
  "src/core/SkIDChangeListener.cpp"
  "src/core/SkImageFilterTypes.cpp"
  "src/core/SkM44.cpp"
  "src/core/SkMalloc.cpp"
  "src/core/SkMarkerStack.cpp"
  "src/core/SkMipmap.cpp"
  "src/core/SkMipmapAccessor.cpp"
  "src/core/SkOpts_erms.cpp"
  "src/core/SkPathBuilder.cpp"
  "src/core/SkRuntimeEffect.cpp"
  "src/core/SkScalerCache.cpp"
  "src/core/SkStrikeForGPU.cpp"
  "src/core/SkTextBlobTrace.cpp"
  "src/core/SkYUVAInfo.cpp"
  "src/core/SkYUVAPixmaps.cpp"
  "src/effects/imagefilters/SkAlphaThresholdImageFilter.cpp"
  "src/effects/imagefilters/SkBlendImageFilter.cpp"
  "src/effects/imagefilters/SkDisplacementMapImageFilter.cpp"
  "src/effects/imagefilters/SkImageImageFilter.cpp"
  "src/effects/imagefilters/SkShaderImageFilter.cpp"
  "src/gpu/GrAATriangulator.cpp"
  "src/gpu/GrAttachment.cpp"
  "src/gpu/GrBackendSemaphore.cpp"
  "src/gpu/GrBackendSurfaceMutableState.cpp"
  "src/gpu/GrBackendUtils.cpp"
  "src/gpu/GrBlockAllocator.cpp"
  "src/gpu/GrClientMappedBufferManager.cpp"
  "src/gpu/GrClipStack.cpp"
  "src/gpu/GrColorInfo.cpp"
  "src/gpu/GrDDLTask.cpp"
  "src/gpu/GrDirectContext.cpp"
  "src/gpu/GrDirectContextPriv.cpp"
  "src/gpu/GrDynamicAtlas.cpp"
  "src/gpu/GrFinishCallbacks.cpp"
  "src/gpu/GrGeometryProcessor.cpp"
  "src/gpu/GrManagedResource.cpp"
  "src/gpu/GrPersistentCacheUtils.cpp"
  "src/gpu/GrProgramInfo.cpp"
  "src/gpu/GrRecordingContextPriv.cpp"
  "src/gpu/GrRectanizerPow2.cpp"
  "src/gpu/GrRectanizerSkyline.cpp"
  "src/gpu/GrRenderTaskCluster.cpp"
  "src/gpu/GrRingBuffer.cpp"
  "src/gpu/GrSPIRVUniformHandler.cpp"
  "src/gpu/GrSPIRVVaryingHandler.cpp"
  "src/gpu/GrStagingBufferManager.cpp"
  "src/gpu/GrStencilMaskHelper.cpp"
  "src/gpu/GrSubRunAllocator.cpp"
  "src/gpu/GrSurfaceDrawContext.cpp"
  "src/gpu/GrSurfaceFillContext.cpp"
  "src/gpu/GrThreadSafeCache.cpp"
  "src/gpu/GrThreadSafePipelineBuilder.cpp"
  "src/gpu/GrTriangulator.cpp"
  "src/gpu/GrUniformDataManager.cpp"
  "src/gpu/GrUtil.cpp"
  "src/gpu/GrWaitRenderTask.cpp"
  "src/gpu/GrWritePixelsRenderTask.cpp"
  "src/gpu/GrYUVABackendTextures.cpp"
  "src/gpu/GrYUVATextureProxies.cpp"
  "src/gpu/SkGpuDevice_nga.cpp"
#"src/gpu/d3d/GrD3DBuffer.cpp"
#"src/gpu/d3d/GrD3DSemaphore.cpp"
#"src/gpu/d3d/GrD3DGpu.cpp"
#"src/gpu/d3d/GrD3DCommandList.cpp"
#"src/gpu/d3d/GrD3DOpsRenderPass.cpp"
#"src/gpu/d3d/GrD3DRootSignature.cpp"
#"src/gpu/d3d/GrD3DDescriptorHeap.cpp"
#"src/gpu/d3d/GrD3DCaps.cpp"
#"src/gpu/d3d/GrD3DUtil.cpp"
#"src/gpu/d3d/GrD3DCpuDescriptorManager.cpp"
#"src/gpu/d3d/GrD3DTypesPriv.cpp"
#"src/gpu/d3d/GrD3DCommandSignature.cpp"
#"src/gpu/d3d/GrD3DPipelineStateBuilder.cpp"
#"src/gpu/d3d/GrD3DTexture.cpp"
#"src/gpu/d3d/GrD3DResourceProvider.cpp"
#"src/gpu/d3d/GrD3DPipelineStateDataManager.cpp"
#"src/gpu/d3d/GrD3DTextureResource.cpp"
#"src/gpu/d3d/GrD3DAMDMemoryAllocator.cpp"
#"src/gpu/d3d/GrD3DDescriptorTableManager.cpp"
#"src/gpu/d3d/GrD3DRenderTarget.cpp"
#"src/gpu/d3d/GrD3DTextureRenderTarget.cpp"
#"src/gpu/d3d/GrD3DAttachment.cpp"
#"src/gpu/d3d/GrD3DPipelineState.cpp"
#"src/gpu/dawn/GrDawnAttachment.cpp"
  "src/gpu/effects/GrBlendFragmentProcessor.cpp"
  "src/gpu/effects/GrMatrixEffect.cpp"
  "src/gpu/effects/GrTextureEffect.cpp"
  "src/gpu/effects/generated/GrArithmeticProcessor.cpp"
  "src/gpu/effects/generated/GrClampFragmentProcessor.cpp"
  "src/gpu/effects/generated/GrDeviceSpaceEffect.cpp"
  "src/gpu/effects/generated/GrDitherEffect.cpp"
  "src/gpu/effects/generated/GrHSLToRGBFilterEffect.cpp"
  "src/gpu/effects/generated/GrRGBToHSLFilterEffect.cpp"
  "src/gpu/geometry/GrStyledShape.cpp"
  "src/gpu/gl/GrGLAttachment.cpp"
#"src/gpu/gl/egl/GrGLMakeEGLInterface.cpp"
#"src/gpu/gl/glx/GrGLMakeGLXInterface.cpp"
#"src/gpu/gl/webgl/GrGLMakeNativeInterface_webgl.cpp"
  "src/gpu/glsl/GrGLSLUniformHandler.cpp"
  "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.cpp"
  "src/gpu/ops/GrSmallPathAtlasMgr.cpp"
  "src/gpu/ops/GrSmallPathShapeData.cpp"
  "src/gpu/ops/GrTriangulatingPathRenderer.cpp"
  "src/gpu/text/GrSDFTControl.cpp"
  "src/gpu/tessellate/GrStrokeInstancedShaderImpl.cpp"
  "src/gpu/tessellate/GrStrokeFixedCountTessellator.cpp"
  "src/gpu/tessellate/GrPathStencilFillOp.cpp"
  "src/gpu/tessellate/GrFillPathShader.cpp"
  "src/gpu/tessellate/GrStrokeHardwareTessellator.cpp"
  "src/gpu/tessellate/GrDrawAtlasPathOp.cpp"
  "src/gpu/tessellate/GrStencilPathShader.cpp"
  "src/gpu/tessellate/GrPathTessellator.cpp"
  "src/gpu/tessellate/GrStrokeShader.cpp"
  "src/gpu/tessellate/GrStrokeTessellateOp.cpp"
  "src/gpu/tessellate/GrStrokeTessellationShaderImpl.cpp"
  "src/gpu/tessellate/GrPathInnerTriangulateOp.cpp"
  "src/gpu/tessellate/GrTessellationPathRenderer.cpp"
  "src/gpu/tessellate/GrStrokeIndirectTessellator.cpp"
#"src/gpu/vk/GrVkAttachment.cpp"
#"src/gpu/vk/GrVkMSAALoadManager.cpp"
  "src/image/SkRescaleAndReadPixels.cpp"
#"src/opts/SkOpts_skx.cpp"
#"src/ports/SkFontMgr_mac_ct.cpp"
#"src/ports/SkFontMgr_mac_ct_factory.cpp"
#"src/ports/SkImageEncoder_NDK.cpp"
#"src/ports/SkImageGeneratorNDK.cpp"
#"src/ports/SkNDKConversions.cpp"
#"src/ports/SkScalerContext_mac_ct.cpp"
#"src/ports/SkTypeface_mac_ct.cpp"
  "src/sksl/SkSLAnalysis.cpp"
  "src/sksl/SkSLBuiltinTypes.cpp"
  "src/sksl/SkSLConstantFolder.cpp"
  "src/sksl/SkSLContext.cpp"
  "src/sksl/SkSLDehydrator.cpp"
  "src/sksl/SkSLInliner.cpp"
  "src/sksl/SkSLMangler.cpp"
  "src/sksl/SkSLOperators.cpp"
  "src/sksl/SkSLPool.cpp"
  "src/sksl/SkSLRehydrator.cpp"
  "src/sksl/SkSLSampleUsage.cpp"
  "src/sksl/codegen/SkSLPipelineStageCodeGenerator.cpp"
  "src/sksl/codegen/SkSLGLSLCodeGenerator.cpp"
  "src/sksl/codegen/SkSLVMCodeGenerator.cpp"
  "src/sksl/codegen/SkSLMetalCodeGenerator.cpp"
  "src/sksl/codegen/SkSLDSLCPPCodeGenerator.cpp"
  "src/sksl/codegen/SkSLCPPCodeGenerator.cpp"
  "src/sksl/codegen/SkSLSPIRVCodeGenerator.cpp"
  "src/sksl/codegen/SkSLSPIRVtoHLSL.cpp"
  "src/sksl/codegen/SkSLHCodeGenerator.cpp"
  "src/sksl/dsl/DSLType.cpp"
  "src/sksl/dsl/DSLCase.cpp"
  "src/sksl/dsl/DSLStatement.cpp"
  "src/sksl/dsl/DSLVar.cpp"
  "src/sksl/dsl/DSLBlock.cpp"
  "src/sksl/dsl/DSLLayout.cpp"
  "src/sksl/dsl/priv/DSLFPs.cpp"
  "src/sksl/dsl/priv/DSLWriter.cpp"
  "src/sksl/dsl/DSLCore.cpp"
  "src/sksl/dsl/DSLRuntimeEffects.cpp"
  "src/sksl/dsl/DSLFunction.cpp"
  "src/sksl/dsl/DSLExpression.cpp"
  "src/sksl/ir/SkSLBinaryExpression.cpp"
  "src/sksl/ir/SkSLBlock.cpp"
  "src/sksl/ir/SkSLConstructor.cpp"
  "src/sksl/ir/SkSLConstructorArray.cpp"
  "src/sksl/ir/SkSLConstructorCompound.cpp"
  "src/sksl/ir/SkSLConstructorCompoundCast.cpp"
  "src/sksl/ir/SkSLConstructorDiagonalMatrix.cpp"
  "src/sksl/ir/SkSLConstructorMatrixResize.cpp"
  "src/sksl/ir/SkSLConstructorScalarCast.cpp"
  "src/sksl/ir/SkSLConstructorSplat.cpp"
  "src/sksl/ir/SkSLConstructorStruct.cpp"
  "src/sksl/ir/SkSLDoStatement.cpp"
  "src/sksl/ir/SkSLExpressionStatement.cpp"
  "src/sksl/ir/SkSLFieldAccess.cpp"
  "src/sksl/ir/SkSLForStatement.cpp"
  "src/sksl/ir/SkSLFunctionCall.cpp"
  "src/sksl/ir/SkSLFunctionDeclaration.cpp"
  "src/sksl/ir/SkSLIfStatement.cpp"
  "src/sksl/ir/SkSLIndexExpression.cpp"
  "src/sksl/ir/SkSLPostfixExpression.cpp"
  "src/sksl/ir/SkSLPrefixExpression.cpp"
  "src/sksl/ir/SkSLSwitchStatement.cpp"
  "src/sksl/ir/SkSLSwizzle.cpp"
  "src/sksl/ir/SkSLTernaryExpression.cpp"
  "src/sksl/ir/SkSLVarDeclarations.cpp"
  "src/sksl/ir/SkSLVariable.cpp"
  "src/utils/SkClipStackUtils.cpp"
  "src/utils/SkCustomTypeface.cpp"
  "src/utils/SkOrderedFontMgr.cpp"
#"src/utils/mac/SkCTFont.cpp"
)
set(SKIA_SRCS ${SKIA_SRCS}
# the following cpp files are of custom features
#
  "custom/SkDropInnerShadowImageFilter.cpp"
)

# add platform-specific codes and compile options
if (NOT EMSCRIPTEN)
  if(VGG_VAR_ARCH STREQUAL "X86")
    set(SKIA_SRCS ${SKIA_SRCS}
      "src/opts/SkOpts_ssse3.cpp"
      "src/opts/SkOpts_sse42.cpp"
      "src/opts/SkOpts_avx.cpp"
      "src/opts/SkOpts_hsw.cpp"
      "src/opts/SkOpts_skx.cpp"
    )
    add_library(skia_base OBJECT ${SKIA_SRCS})
    target_compile_options(skia_base PRIVATE
      $<$<CONFIG:Debug>:-DSK_DEBUG>
      $<$<CONFIG:Release>:-DSK_RELEASE -fstrict-aliasing -fvisibility=hidden -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -fno-exceptions -frtti>
      -mssse3
      -msse4.2
      -mavx
    )
  else() # ARM
    set(SKIA_SRCS ${SKIA_SRCS}
      "src/opts/SkOpts_crc32.cpp"
    )
    add_library(skia_base OBJECT ${SKIA_SRCS})
    target_compile_options(skia_base PRIVATE
      $<$<CONFIG:Debug>:-DSK_DEBUG>
      $<$<CONFIG:Release>:-DSK_RELEASE -fstrict-aliasing -fvisibility=hidden -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -fno-exceptions -frtti>
    )
  endif()
else()
  add_library(skia_base OBJECT ${SKIA_SRCS})
  target_compile_options(skia_base PRIVATE
    -DSK_ASSUME_WEBGL=1
    -DSK_USE_WEBGL
    -DSK_FORCE_8_BYTE_ALIGNMENT
    $<$<CONFIG:Debug>:-DSK_DEBUG>
    $<$<CONFIG:Release>:-DSK_RELEASE -fstrict-aliasing -fdata-sections -ffunction-sections -fno-exceptions -frtti>
  )
endif()
set_target_properties(skia_base PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(skia_base PRIVATE
  # deprecated in m92:
  #
  #SK_BUILD_NO_OPTS=1 # disable optimization like avx/sse/hsw etc in cpu rasterizer
  #
  #SK_HAS_PNG_LIBRARY=1
  #SK_HAS_JPEG_LIBRARY
  #SK_HAS_WEBP_LIBRARY
  #
  #SK_DISABLE_READBUFFER=1 # disable serialization to/from SkPicture (i.e. skp files)

  SKIA_DLL=1         # turn on visibility of APIs flaged with SK_API
  SK_DISABLE_AAA=1   # disable anti-alias algorithm in cpu rasterizer
  SK_GL

  # disable serialization to/from SkPicture (i.e. skp files)
  SK_DISABLE_EFFECT_DESERIALIZATION=1

  # disable low-precision software rendering
  SK_DISABLE_LEGACY_SHADERCONTEXT=1
  SK_DISABLE_LOWP_RASTER_PIPELINE=1
  SK_FORCE_RASTER_PIPELINE_BLITTER=1

  # image codecs
  SK_CODEC_DECODES_PNG
  SK_CODEC_DECODES_JPEG
  SK_CODEC_DECODES_WEBP
  SK_ENCODE_PNG
  SK_ENCODE_JPEG
  SK_ENCODE_WEBP
  SK_USE_LIBGIFCODEC
)
# setting flags for individual files
# NOTE changing flags below will cause entire rebuild of skia
#      using make generator, while ninja has no such problem.
set_source_files_properties(
  "src/ports/SkFontHost_FreeType.cpp"
  PROPERTIES
  COMPILE_FLAGS -DSK_FREETYPE_MINIMUM_RUNTIME_VERSION=0x020a0100
)
set_source_files_properties(
  "src/ports/SkFontMgr_custom_directory_factory.cpp"
  PROPERTIES
  COMPILE_FLAGS -DSK_FONT_FILE_PREFIX=\\".\\"
)


if (VGG_VAR_HOST STREQUAL "macOS")
  set(SKIA_INTERFACE_SRCS
    #"custom/GrGLMakeNativeInterface_angle.cpp"
    "src/gpu/gl/mac/GrGLMakeNativeInterface_mac.cpp"
  )
elseif(VGG_VAR_HOST STREQUAL "Linux")
  set(SKIA_INTERFACE_SRCS
    "src/gpu/gl/glx/GrGLMakeNativeInterface_glx.cpp"
    "src/gpu/gl/glx/GrGLMakeGLXInterface.cpp"
  )
elseif(VGG_VAR_HOST STREQUAL "Windows")
  set(SKIA_INTERFACE_SRCS
    "src/gpu/gl/win/GrGLMakeNativeInterface_win.cpp"
  )
elseif(VGG_VAR_HOST STREQUAL "iOS")
  set(SKIA_INTERFACE_SRCS
    "src/gpu/gl/iOS/GrGLMakeNativeInterface_iOS.cpp"
  )
elseif(VGG_VAR_HOST STREQUAL "Android")
  set(SKIA_INTERFACE_SRCS
    "src/gpu/gl/android/GrGLMakeNativeInterface_android.cpp"
  )
elseif(VGG_VAR_HOST STREQUAL "WASM")
  set(SKIA_INTERFACE_SRCS
    "src/gpu/gl/webgl/GrGLMakeNativeInterface_webgl.cpp"
  )
endif()

add_library(skia STATIC ${SKIA_INTERFACE_SRCS})
target_link_libraries(skia $<TARGET_OBJECTS:skia_base> zlib libpng freetype pthread libgifcodec libjpeg-turbo libwebp)
target_compile_options(skia PRIVATE
  $<$<CONFIG:Debug>:-DSK_DEBUG>
  $<$<CONFIG:Release>:-DSK_RELEASE -fstrict-aliasing -fvisibility=hidden -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -fno-exceptions -frtti>
)

if(VGG_ENABLE_EGL)
  set(SKIA_EGL_INTERFACE_SRCS
    "src/gpu/gl/egl/GrGLMakeNativeInterface_egl.cpp"
    "src/gpu/gl/egl/GrGLMakeEGLInterface.cpp"
  )
  add_library(skia_egl SHARED ${SKIA_EGL_INTERFACE_SRCS})
  target_link_libraries(skia_egl $<TARGET_OBJECTS:skia_base> zlib libpng freetype pthread libgifcodec libjpeg-turbo libwebp OpenGL::EGL)
  target_compile_options(skia_egl PRIVATE
    $<$<CONFIG:Debug>:-DSK_DEBUG>
    $<$<CONFIG:Release>:-DSK_RELEASE -fstrict-aliasing -fvisibility=hidden -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -fno-exceptions -frtti>
  )
endif()
