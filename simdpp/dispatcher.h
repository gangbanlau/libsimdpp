/*  libsimdpp
    Copyright (C) 2013  Povilas Kanapickas tir5c3@yahoo.co.uk
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBSIMDPP_DISPATCHER_H
#define LIBSIMDPP_DISPATCHER_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <functional>
#include <simdpp/arch.h>

namespace simdpp {

/** @defgroup simd_dispatcher Dispatching support
    @{
*/

using GetArchCb = std::function<Arch()>;

/** @def SIMDPP_USER_ARCH_INFO
    The user must define this macro if he wants to use the dispatcher
    infrastructure. The macro must be defined before a SIMDPP_MAKE_DISPATCHER_*
    function is used. All SIMDPP_MAKE_DISPATCHER_* usage sites must see the same
    definition of the macro. The macro must evaluate to a constant expression
    that could implicitly initialize an object of type @c std::function<Arch()>.

    The function is called at unspecified time to determine what features are
    supported by the processor.

    The user must ensure that the returned information is sensible: e.g. SSE2
    must be supported if SSE3 support is indicated.

    The @c simdpp/dispatch/get_arch_*.h files provide several ready
    implementations of CPU features detection.
*/

/// @} -- end defgroup

} // namespace simdpp

#include <simdpp/detail/dispatcher.h>
#include <simdpp/dispatcher_macros.h>

#endif
