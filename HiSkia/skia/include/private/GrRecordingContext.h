/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContext_DEFINED
#define GrRecordingContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrImageContext.h"

class GrAuditTrail;
class GrBackendFormat;
class GrDrawingManager;
class GrOnFlushCallbackObject;
class GrOpMemoryPool;
class GrProgramInfo;
class GrRecordingContextPriv;
class GrStrikeCache;
class GrSurfaceContext;
class GrSurfaceProxy;
class GrTextBlobCache;
class SkArenaAlloc;

class GrRecordingContext : public GrImageContext {
public:
    ~GrRecordingContext() override;

    SK_API GrBackendFormat defaultBackendFormat(SkColorType ct, GrRenderable renderable) const {
        return INHERITED::defaultBackendFormat(ct, renderable);
    }

    // Provides access to functions that aren't part of the public API.
    GrRecordingContextPriv priv();
    const GrRecordingContextPriv priv() const;

#if GR_TEST_UTILS
    // Used by tests that induce intentional allocation failures, in order to keep the output clean.
    void testingOnly_setSuppressAllocationWarnings() { fSuppressAllocationWarnings = true; }
    bool testingOnly_getSuppressAllocationWarnings() const { return fSuppressAllocationWarnings; }
#endif

    // The collection of specialized memory arenas for different types of data recorded by a
    // GrRecordingContext. Arenas does not maintain ownership of the pools it groups together.
    class Arenas {
    public:
        Arenas(GrOpMemoryPool*, SkArenaAlloc*);

        // For storing GrOp-derived classes recorded by a GrRecordingContext
        GrOpMemoryPool* opMemoryPool() { return fOpMemoryPool; }

        // For storing pipelines and other complex data as-needed by ops
        SkArenaAlloc* recordTimeAllocator() { return fRecordTimeAllocator; }

    private:
        GrOpMemoryPool* fOpMemoryPool;
        SkArenaAlloc*   fRecordTimeAllocator;
    };

protected:
    friend class GrRecordingContextPriv; // for hidden functions
    friend class SkDeferredDisplayList;  // for OwnedArenas;

    // Like Arenas, but preserves ownership of the underlying pools.
    class OwnedArenas {
    public:
        OwnedArenas();
        ~OwnedArenas();

        Arenas get();

        OwnedArenas& operator=(OwnedArenas&&);

    private:
        std::unique_ptr<GrOpMemoryPool> fOpMemoryPool;
        std::unique_ptr<SkArenaAlloc>   fRecordTimeAllocator;
    };

    GrRecordingContext(GrBackendApi, const GrContextOptions&, uint32_t contextID);
    bool init(sk_sp<const GrCaps>) override;
    void setupDrawingManager(bool sortOpsTasks, bool reduceOpsTaskSplitting);

    void abandonContext() override;

    GrDrawingManager* drawingManager();

    Arenas arenas() { return fArenas.get(); }
    // This entry point should only be used for DDL creation where we want the ops' lifetime to
    // match that of the DDL.
    OwnedArenas&& detachArenas();

    // This entry point gives the recording context a chance to cache the provided
    // programInfo. The DDL context takes this opportunity to store programInfos as a sidecar
    // to the DDL.
    virtual void recordProgramInfo(const GrProgramInfo*) {}
    // This asks the recording context to return any programInfos it may have collected
    // via the 'recordProgramInfo' call. It is up to the caller to ensure that the lifetime
    // of the programInfos matches the intended use. For example, in DDL-record mode it
    // is known that all the programInfos will have been allocated in an arena with the
    // same lifetime at the DDL itself.
    virtual void detachProgramInfos(SkTDArray<const GrProgramInfo*>*) {}

    GrStrikeCache* getGrStrikeCache() { return fStrikeCache.get(); }
    GrTextBlobCache* getTextBlobCache();
    const GrTextBlobCache* getTextBlobCache() const;

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    /*
     * Create a new render target context backed by a deferred-style
     * GrRenderTargetProxy. We guarantee that "asTextureProxy" will succeed for
     * renderTargetContexts created via this entry point.
     */
    std::unique_ptr<GrRenderTargetContext> makeDeferredRenderTargetContext(
            SkBackingFit fit,
            int width,
            int height,
            GrColorType colorType,
            sk_sp<SkColorSpace> colorSpace,
            int sampleCnt = 1,
            GrMipMapped = GrMipMapped::kNo,
            GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
            const SkSurfaceProps* surfaceProps = nullptr,
            SkBudgeted = SkBudgeted::kYes,
            GrProtected isProtected = GrProtected::kNo);

    /*
     * This method will attempt to create a renderTargetContext that has, at least, the number of
     * channels and precision per channel as requested in 'config' (e.g., A8 and 888 can be
     * converted to 8888). It may also swizzle the channels (e.g., BGRA -> RGBA).
     * SRGB-ness will be preserved.
     */
    std::unique_ptr<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
            SkBackingFit fit,
            int width,
            int height,
            GrColorType colorType,
            sk_sp<SkColorSpace> colorSpace,
            int sampleCnt = 1,
            GrMipMapped = GrMipMapped::kNo,
            GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
            const SkSurfaceProps* surfaceProps = nullptr,
            SkBudgeted budgeted = SkBudgeted::kYes,
            GrProtected isProtected = GrProtected::kNo);

    GrAuditTrail* auditTrail() { return fAuditTrail.get(); }

    GrRecordingContext* asRecordingContext() override { return this; }

private:
    OwnedArenas                       fArenas;

    std::unique_ptr<GrDrawingManager> fDrawingManager;

    std::unique_ptr<GrStrikeCache>    fStrikeCache;
    std::unique_ptr<GrTextBlobCache>  fTextBlobCache;

    std::unique_ptr<GrAuditTrail>     fAuditTrail;

#ifdef GR_TEST_UTILS
    bool fSuppressAllocationWarnings = false;
#endif

    typedef GrImageContext INHERITED;
};

#endif
