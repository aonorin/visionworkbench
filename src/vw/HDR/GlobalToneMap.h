// __BEGIN_LICENSE__
// 
// Copyright (C) 2006 United States Government as represented by the
// Administrator of the National Aeronautics and Space Administration
// (NASA).  All Rights Reserved.
// 
// Copyright 2006 Carnegie Mellon University. All rights reserved.
// 
// This software is distributed under the NASA Open Source Agreement
// (NOSA), version 1.3.  The NOSA has been approved by the Open Source
// Initiative.  See the file COPYING at the top of the distribution
// directory tree for the complete NOSA document.
// 
// THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY
// KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT
// LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO
// SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
// A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT
// THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT
// DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE.
// 
// __END_LICENSE__

/// \file GlobalToneMap.h
/// 
/// Tonemapping operators that operate on the entire image.  That is,
/// the tonemapped value of each pixel can be computed without knowing
/// the values of pixels in its immediate neighborhood.
///
/// The tonemapping operators currently defined in this file include:
///
/// - Drago Logarithmic Tonemap Operator
/// 

#ifndef __VW_HDR_GLOBALTONEMAP_H__
#define __VW_HDR_GLOBALTONEMAP_H__

#include <vw/Image/ImageView.h>
#include <vw/Image/PixelTypes.h>
#include <vw/Image/PixelTypeInfo.h>
#include <vw/Image/PerPixelViews.h>
#include <vw/Image/Filter.h>

namespace vw { 
namespace hdr {

  const float DRAGO_DEFAULT_BIAS = 0.85;

  template <class PixelT>
  class DragoFunctor : public vw::ReturnFixedType<typename CompoundChannelCast<PixelT,double>::type> {
  private:
    double L_wmax, power, offset, scale;
    int b_min, b_diff;
  public:

    DragoFunctor(double L_wmin, double L_wmax, double bias, int b_min, int b_max) : 
      L_wmax(L_wmax), b_min(b_min) {
      b_diff = b_max - b_min;

      offset = log(L_wmin + 1.0) / log(b_min + b_diff * pow(L_wmin / L_wmax, power));
      scale = log(L_wmax + 1.0) / log(b_min + b_diff * pow(L_wmax / L_wmax, power)) - offset;

      power = log(bias) / log(0.5);
    }
    
    typename CompoundChannelCast<PixelT,double>::type operator() (PixelT L_w) const {
      return (log(L_w + 1.0) / log(b_min + b_diff * pow(L_w / L_wmax, power)) - offset) / scale;
    }
  };
  
  template <class ViewT>
  UnaryPerPixelView<ViewT, DragoFunctor<typename ViewT::pixel_type> > drago_tone_map(ImageViewBase<ViewT> const& hdr_image, double bias) {
    
    // Compute range of luminances
    double L_wmax, L_wmin;
    min_max_channel_values(pixel_cast<PixelGray<double> >(hdr_image), L_wmin, L_wmax);

    // Create the tone mapping functor and return a UnaryPerPixelView
    //
    // These constants are recommended in Drago's paper -- they seem
    // to provide adequate highlight compression while still
    // preserving contrast and detail.
    const int DRAGO_MIN_BASE = 2;
    const int DRAGO_MAX_BASE = 10;

    DragoFunctor<typename ViewT::pixel_type> drago_tm(L_wmin, L_wmax, bias, DRAGO_MIN_BASE, DRAGO_MAX_BASE);
    return per_pixel_filter(hdr_image, drago_tm);
  }

}} // namespace vw::HDR


#endif  // __GlobalToneMap_H__
