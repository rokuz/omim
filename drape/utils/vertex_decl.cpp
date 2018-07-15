#include "drape/utils/vertex_decl.hpp"

namespace gpu
{
namespace
{
enum VertexType
{
  Area,
  Area3d,
  HatchingArea,
  SolidTexturing,
  MaskedTexturing,
  TextStatic,
  TextOutlinedStatic,
  TextDynamic,
  Line,
  DashedLine,
  Route,
  RouteMarker,
  ColoredSymbol,
  TypeCount
};

struct BindingNode
{
  dp::BindingInfo m_info;
  bool m_inited = false;
};

typedef dp::BindingInfo (*TInitFunction)();

dp::BindingInfo AreaBindingInit()
{
  static_assert(sizeof(AreaVertex) == (sizeof(AreaVertex::TPosition) +
                                       sizeof(AreaVertex::TPackedColor)), "");

  dp::BindingFiller<AreaVertex> filler(2);
  filler.FillDecl<AreaVertex::TPosition>("a_position");
  filler.FillDecl<AreaVertex::TPackedColor>("a_packedColor");

  return filler.m_info;
}

dp::BindingInfo Area3dBindingInit()
{
  static_assert(sizeof(Area3dVertex) == (sizeof(Area3dVertex::TPosition) +
                                         sizeof(Area3dVertex::TNormal3d) +
                                         sizeof(Area3dVertex::TPackedColor)), "");

  dp::BindingFiller<Area3dVertex> filler(3);
  filler.FillDecl<Area3dVertex::TPosition>("a_position");
  filler.FillDecl<Area3dVertex::TNormal3d>("a_normal");
  filler.FillDecl<Area3dVertex::TPackedColor>("a_packedColor");

  return filler.m_info;
}

dp::BindingInfo HatchingAreaBindingInit()
{
  static_assert(sizeof(HatchingAreaVertex) == (sizeof(HatchingAreaVertex::TPosition) +
                                               sizeof(HatchingAreaVertex::TPackedColor) +
                                               sizeof(HatchingAreaVertex::TMaskTexCoord)), "");

  dp::BindingFiller<HatchingAreaVertex> filler(3);
  filler.FillDecl<HatchingAreaVertex::TPosition>("a_position");
  filler.FillDecl<HatchingAreaVertex::TPackedColor>("a_packedColor");
  filler.FillDecl<HatchingAreaVertex::TMaskTexCoord>("a_maskTexCoords");
  return filler.m_info;
}

dp::BindingInfo SolidTexturingBindingInit()
{
  static_assert(sizeof(SolidTexturingVertex) == (sizeof(SolidTexturingVertex::TPosition3d) +
                                                 sizeof(SolidTexturingVertex::TNormal) +
                                                 sizeof(SolidTexturingVertex::TTexCoord)), "");

  dp::BindingFiller<SolidTexturingVertex> filler(3);
  filler.FillDecl<SolidTexturingVertex::TPosition3d>("a_position");
  filler.FillDecl<SolidTexturingVertex::TNormal>("a_normal");
  filler.FillDecl<SolidTexturingVertex::TTexCoord>("a_texCoords");

  return filler.m_info;
}

dp::BindingInfo MaskedTexturingBindingInit()
{
  static_assert(sizeof(MaskedTexturingVertex) == (sizeof(MaskedTexturingVertex::TPosition3d) +
                                                  sizeof(MaskedTexturingVertex::TNormal) +
                                                  sizeof(MaskedTexturingVertex::TTexCoord) +
                                                  sizeof(MaskedTexturingVertex::TPackedColor)), "");

  dp::BindingFiller<MaskedTexturingVertex> filler(4);
  filler.FillDecl<MaskedTexturingVertex::TPosition3d>("a_position");
  filler.FillDecl<MaskedTexturingVertex::TNormal>("a_normal");
  filler.FillDecl<MaskedTexturingVertex::TTexCoord>("a_texCoords");
  filler.FillDecl<MaskedTexturingVertex::TPackedColor>("a_packedColor");

  return filler.m_info;
}

dp::BindingInfo TextStaticBindingInit()
{
  static_assert(sizeof(TextStaticVertex) == (sizeof(TextStaticVertex::TPackedColor) +
                                             sizeof(TextStaticVertex::TTexCoord)), "");

  dp::BindingFiller<TextStaticVertex> filler(2);
  filler.FillDecl<TextStaticVertex::TPackedColor>("a_packedColor");
  filler.FillDecl<TextStaticVertex::TTexCoord>("a_maskTexCoord");

  return filler.m_info;
}

dp::BindingInfo TextOutlinedStaticBindingInit()
{
  static_assert(sizeof(TextOutlinedStaticVertex) == (2 * sizeof(TextStaticVertex::TPackedColor) +
                                                     sizeof(TextOutlinedStaticVertex::TTexCoord)), "");

  dp::BindingFiller<TextOutlinedStaticVertex> filler(3);
  filler.FillDecl<TextOutlinedStaticVertex::TPackedColor>("a_packedColor");
  filler.FillDecl<TextOutlinedStaticVertex::TPackedColor>("a_packedOutlineColor");
  filler.FillDecl<TextOutlinedStaticVertex::TTexCoord>("a_maskTexCoord");

  return filler.m_info;
}

dp::BindingInfo TextDynamicBindingInit()
{
  static_assert(sizeof(TextDynamicVertex) == (sizeof(TextStaticVertex::TPosition3d) +
                                              sizeof(TextDynamicVertex::TNormal)), "");

  dp::BindingFiller<TextDynamicVertex> filler(2, TextDynamicVertex::GetDynamicStreamID());
  filler.FillDecl<TextStaticVertex::TPosition3d>("a_position");
  filler.FillDecl<TextDynamicVertex::TNormal>("a_normal");

  return filler.m_info;
}

dp::BindingInfo LineBindingInit()
{
  static_assert(sizeof(LineVertex) == sizeof(LineVertex::TPosition) +
                                      sizeof(LineVertex::TNormal) +
                                      sizeof(LineVertex::TPackedColor), "");
  dp::BindingFiller<LineVertex> filler(3);
  filler.FillDecl<LineVertex::TPosition>("a_position");
  filler.FillDecl<LineVertex::TNormal>("a_normal");
  filler.FillDecl<LineVertex::TPackedColor>("a_packedColor");

  return filler.m_info;
}

dp::BindingInfo DashedLineBindingInit()
{
  static_assert(sizeof(DashedLineVertex) == sizeof(DashedLineVertex::TPosition) +
                                            sizeof(DashedLineVertex::TNormal) +
                                            sizeof(DashedLineVertex::TPackedColor) +
                                            sizeof(DashedLineVertex::TMaskTexCoord), "");

  dp::BindingFiller<DashedLineVertex> filler(4);
  filler.FillDecl<DashedLineVertex::TPosition>("a_position");
  filler.FillDecl<DashedLineVertex::TNormal>("a_normal");
  filler.FillDecl<DashedLineVertex::TPackedColor>("a_packedColor");
  filler.FillDecl<DashedLineVertex::TMaskTexCoord>("a_maskTexCoord");

  return filler.m_info;
}

dp::BindingInfo RouteBindingInit()
{
  static_assert(sizeof(RouteVertex) == sizeof(RouteVertex::TPosition) +
                                       sizeof(RouteVertex::TNormal) +
                                       sizeof(RouteVertex::TLength) +
                                       sizeof(RouteVertex::TColor), "");

  dp::BindingFiller<RouteVertex> filler(4);
  filler.FillDecl<RouteVertex::TPosition>("a_position");
  filler.FillDecl<RouteVertex::TNormal>("a_normal");
  filler.FillDecl<RouteVertex::TLength>("a_length");
  filler.FillDecl<RouteVertex::TColor>("a_color");

  return filler.m_info;
}

dp::BindingInfo RouteMarkerBindingInit()
{
  static_assert(sizeof(RouteMarkerVertex) == sizeof(RouteMarkerVertex::TPosition) +
                                             sizeof(RouteMarkerVertex::TNormal) +
                                             sizeof(RouteMarkerVertex::TColor), "");

  dp::BindingFiller<RouteMarkerVertex> filler(3);
  filler.FillDecl<RouteMarkerVertex::TPosition>("a_position");
  filler.FillDecl<RouteMarkerVertex::TNormal>("a_normal");
  filler.FillDecl<RouteMarkerVertex::TColor>("a_color");

  return filler.m_info;
}

dp::BindingInfo ColoredSymbolBindingInit()
{
  static_assert(sizeof(ColoredSymbolVertex) == sizeof(ColoredSymbolVertex::TPosition) +
                                               sizeof(ColoredSymbolVertex::TNormal) +
                                               sizeof(ColoredSymbolVertex::TPackedColor) +
                                               sizeof(ColoredSymbolVertex::TOffset), "");

  dp::BindingFiller<ColoredSymbolVertex> filler(4);
  filler.FillDecl<ColoredSymbolVertex::TPosition>("a_position");
  filler.FillDecl<ColoredSymbolVertex::TNormal>("a_normal");
  filler.FillDecl<ColoredSymbolVertex::TPackedColor>("a_packedColor");
  filler.FillDecl<ColoredSymbolVertex::TOffset>("a_offset");

  return filler.m_info;
}

BindingNode g_bindingNodes[TypeCount];
TInitFunction g_initFunctions[TypeCount] =
{
  &AreaBindingInit,
  &Area3dBindingInit,
  &HatchingAreaBindingInit,
  &SolidTexturingBindingInit,
  &MaskedTexturingBindingInit,
  &TextStaticBindingInit,
  &TextOutlinedStaticBindingInit,
  &TextDynamicBindingInit,
  &LineBindingInit,
  &DashedLineBindingInit,
  &RouteBindingInit,
  &RouteMarkerBindingInit,
  &ColoredSymbolBindingInit
};

dp::BindingInfo const & GetBinding(VertexType type)
{
  BindingNode & node = g_bindingNodes[type];
  if (!node.m_inited)
  {
    node.m_info = g_initFunctions[type]();
    node.m_inited = true;
  }

  return node.m_info;
}
}  // namespace

AreaVertex::AreaVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_packedColor(0.0, 0.0)
{}

AreaVertex::AreaVertex(TPosition const & position, TPackedColor const & packedColor)
  : m_position(position)
  , m_packedColor(packedColor)
{}

dp::BindingInfo const & AreaVertex::GetBindingInfo()
{
  return GetBinding(Area);
}

Area3dVertex::Area3dVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0, 0.0)
  , m_packedColor(0.0, 0.0)
{}

Area3dVertex::Area3dVertex(TPosition const & position, const TPosition & normal,
                           TPackedColor const & packedColor)
  : m_position(position)
  , m_normal(normal)
  , m_packedColor(packedColor)
{}

dp::BindingInfo const & Area3dVertex::GetBindingInfo()
{
  return GetBinding(Area3d);
}

HatchingAreaVertex::HatchingAreaVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_packedColor(0.0, 0.0)
  , m_maskTexCoord(0.0, 0.0)
{}

HatchingAreaVertex::HatchingAreaVertex(TPosition const & position, TPackedColor const & packedColor,
                                       TMaskTexCoord const & maskTexCoord)
  : m_position(position)
  , m_packedColor(packedColor)
  , m_maskTexCoord(maskTexCoord)
{}

dp::BindingInfo const & HatchingAreaVertex::GetBindingInfo()
{
  return GetBinding(HatchingArea);
}

SolidTexturingVertex::SolidTexturingVertex()
  : m_position(0.0, 0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0)
  , m_texCoord(0.0, 0.0)
{}

SolidTexturingVertex::SolidTexturingVertex(TPosition3d const & position, TNormal const & normal,
                                           TTexCoord const & texCoord)
  : m_position(position)
  , m_normal(normal)
  , m_texCoord(texCoord)
{}

dp::BindingInfo const & SolidTexturingVertex::GetBindingInfo()
{
  return GetBinding(SolidTexturing);
}

MaskedTexturingVertex::MaskedTexturingVertex()
  : m_position(0.0, 0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0)
  , m_texCoord(0.0, 0.0)
  , m_maskColor(0.0, 0.0)
{}

MaskedTexturingVertex::MaskedTexturingVertex(TPosition3d const & position, TNormal const & normal,
                                             TTexCoord const & texCoord, TPackedColor const & maskColor)
  : m_position(position)
  , m_normal(normal)
  , m_texCoord(texCoord)
  , m_maskColor(maskColor)
{}

dp::BindingInfo const & MaskedTexturingVertex::GetBindingInfo()
{
  return GetBinding(MaskedTexturing);
}

TextOutlinedStaticVertex::TextOutlinedStaticVertex()
  : m_packedColor(0.0, 0.0)
  , m_packedOutlineColor(0.0, 0.0)
  , m_maskTexCoord(0.0, 0.0)
{}

  TextOutlinedStaticVertex::TextOutlinedStaticVertex(TPackedColor const & packedColor,
                                                     TPackedColor const & packedOutlineColor,
                                                     TTexCoord const & maskTexCoord)
  : m_packedColor(packedColor)
  , m_packedOutlineColor(packedOutlineColor)
  , m_maskTexCoord(maskTexCoord)
{}

dp::BindingInfo const & TextOutlinedStaticVertex::GetBindingInfo()
{
  return GetBinding(TextOutlinedStatic);
}

TextDynamicVertex::TextDynamicVertex()
  : m_position(0.0, 0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0)
{}

TextDynamicVertex::TextDynamicVertex(const TPosition3d & position, TNormal const & normal)
  : m_position(position),
    m_normal(normal)
{}

dp::BindingInfo const & TextDynamicVertex::GetBindingInfo()
{
  return GetBinding(TextDynamic);
}

uint32_t TextDynamicVertex::GetDynamicStreamID()
{
  return 0x7F;
}

LineVertex::LineVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0, 0.0)
  , m_packedColor(0.0, 0.0)
{}

LineVertex::LineVertex(TPosition const & position, TNormal const & normal,
                       TPackedColor const & color)
  : m_position(position)
  , m_normal(normal)
  , m_packedColor(color)
{}

dp::BindingInfo const & LineVertex::GetBindingInfo()
{
  return GetBinding(Line);
}

DashedLineVertex::DashedLineVertex()
  : m_position(0.0f, 0.0f, 0.0f)
  , m_normal(0.0f, 0.0f, 0.0f)
  , m_packedColor(0.0f, 0.0f)
  , m_maskTexCoord(0.0f, 0.0f, 0.0f, 0.0f)
{}

DashedLineVertex::DashedLineVertex(TPosition const & position, TNormal const & normal,
                                   TPackedColor const & color, TMaskTexCoord const & mask)
  : m_position(position)
  , m_normal(normal)
  , m_packedColor(color)
  , m_maskTexCoord(mask)
{}

dp::BindingInfo const & DashedLineVertex::GetBindingInfo()
{
  return GetBinding(DashedLine);
}

RouteVertex::RouteVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0)
  , m_length(0.0, 0.0, 0.0)
  , m_color(0.0, 0.0, 0.0, 0.0)
{}

RouteVertex::RouteVertex(TPosition const & position, TNormal const & normal,
                         TLength const & length, TColor const & color)
  : m_position(position)
  , m_normal(normal)
  , m_length(length)
  , m_color(color)
{}

dp::BindingInfo const & RouteVertex::GetBindingInfo()
{
  return GetBinding(Route);
}

RouteMarkerVertex::RouteMarkerVertex()
  : m_position(0.0, 0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0, 0.0)
  , m_color(0.0, 0.0, 0.0, 0.0)
{}

RouteMarkerVertex::RouteMarkerVertex(TPosition const & position, TNormal const & normal,
                                     TColor const & color)
  : m_position(position)
  , m_normal(normal)
  , m_color(color)
{}

dp::BindingInfo const & RouteMarkerVertex::GetBindingInfo()
{
  return GetBinding(RouteMarker);
}

TextStaticVertex::TextStaticVertex()
  : m_packedColor(0.0, 0.0)
  , m_maskTexCoord(0.0, 0.0)
{}

TextStaticVertex::TextStaticVertex(TPackedColor const & packedColor, TTexCoord const & maskTexCoord)
  : m_packedColor(packedColor)
  , m_maskTexCoord(maskTexCoord)
{}

dp::BindingInfo const & TextStaticVertex::GetBindingInfo()
{
  return GetBinding(TextStatic);
}

ColoredSymbolVertex::ColoredSymbolVertex()
  : m_position(0.0, 0.0, 0.0)
  , m_normal(0.0, 0.0, 0.0, 0.0)
  , m_packedColor(0.0, 0.0)
  , m_offset(0.0, 0.0)
{}

ColoredSymbolVertex::ColoredSymbolVertex(TPosition const & position, TNormal const & normal,
                                         TPackedColor const & packedColor, TOffset const & offset)
  : m_position(position)
  , m_normal(normal)
  , m_packedColor(packedColor)
  , m_offset(offset)
{}

dp::BindingInfo const & ColoredSymbolVertex::GetBindingInfo()
{
  return GetBinding(ColoredSymbol);
}
}  // namespace gpu

