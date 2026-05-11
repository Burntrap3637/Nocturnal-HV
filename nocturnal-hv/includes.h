#pragma once
#include <intrin.h>

#include <deps/types.h>
#include <deps/intrinsic.hxx>
#include <deps/ia32/ia32.h>
#include <deps/hv/hv.h>
#include <deps/entry.h>

#include <ext/oxorany/include.h>

#include <core/host/kernel/kernel.hxx>
#include <core/host/kernel/routines.hxx>

#include <core/utility/utility.hxx>
#include <core/host/memory/memory.hxx>
#include <core/host/memory/physical/physical.hxx>
#include <core/host/memory/paging/paging.hxx>

#include <core/guest/guest.h>
#include <core/guest/svm/npt/npt.hxx>
#include <core/guest/svm/vmcb/vmcb.hxx>
#include <core/guest/svm/vmexit/vmexit.hxx>
#include <core/guest/svm/svm.hxx>

#include <core/host/power/power.hxx>