#include "drape_frontend/area_shape.hpp"

#include "drape/shader_def.hpp"
#include "drape/glstate.hpp"
#include "drape/batcher.hpp"
#include "drape/attribute_provider.hpp"
#include "drape/texture_manager.hpp"
#include "drape/utils/vertex_decl.hpp"

#include "indexer/map_style_reader.hpp"

#include "base/buffer_vector.hpp"
#include "base/logging.hpp"

#include "std/algorithm.hpp"

namespace
{

float const kLightOutlineColorFactor = 0.8;
float const kDarkOutlineColorFactor = 1.4;

} // namespace

namespace df
{

AreaShape::AreaShape(vector<m2::PointD> && triangleList, BuildingOutline && buildingOutline,
                     AreaViewParams const & params)
  : m_vertexes(move(triangleList))
  , m_buildingOutline(move(buildingOutline))
  , m_params(params)
{}

void AreaShape::Draw(ref_ptr<dp::Batcher> batcher, ref_ptr<dp::TextureManager> textures) const
{
  auto const style = GetStyleReader().GetCurrentStyle();
  float const colorFactor = (style == MapStyleDark) ? kDarkOutlineColorFactor : kLightOutlineColorFactor;

  dp::TextureManager::ColorRegion region;
  textures->GetColorRegion(m_params.m_color, region);
  dp::TextureManager::ColorRegion outlineRegion;
  textures->GetColorRegion(m_params.m_color * colorFactor, outlineRegion);
  ASSERT_EQUAL(region.GetTexture(), outlineRegion.GetTexture(), ());

  if (m_params.m_is3D)
    DrawArea3D(batcher, region.GetTexRect().Center(), outlineRegion.GetTexRect().Center(), region.GetTexture());
  else
    DrawArea(batcher, region.GetTexRect().Center(), outlineRegion.GetTexRect().Center(), region.GetTexture());
}

void AreaShape::DrawArea(ref_ptr<dp::Batcher> batcher, m2::PointD const & colorUv, m2::PointD const & outlineUv,
                         ref_ptr<dp::Texture> texture) const
{
  glsl::vec2 const uv = glsl::ToVec2(colorUv);

  buffer_vector<gpu::AreaVertex, 128> vertexes;
  vertexes.resize(m_vertexes.size());
  transform(m_vertexes.begin(), m_vertexes.end(), vertexes.begin(), [&uv, this](m2::PointF const & vertex)
  {
    return gpu::AreaVertex(glsl::vec3(glsl::ToVec2(ConvertToLocal(vertex, m_params.m_tileCenter, kShapeCoordScalar)),
                                      m_params.m_depth), uv);
  });

  dp::GLState state(gpu::AREA_PROGRAM, dp::GLState::GeometryLayer);
  state.SetColorTexture(texture);

  dp::AttributeProvider provider(1, m_vertexes.size());
  provider.InitStream(0, gpu::AreaVertex::GetBindingInfo(), make_ref(vertexes.data()));
  batcher->InsertTriangleList(state, make_ref(&provider));

  // Generate outline.
  if (!m_buildingOutline.m_indices.empty())
  {
    glsl::vec2 const ouv = glsl::ToVec2(outlineUv);

    vector<gpu::AreaVertex> vertices;
    vertices.reserve(m_buildingOutline.m_vertices.size());
    for (size_t i = 0; i < m_buildingOutline.m_vertices.size(); i++)
    {
      glsl::vec2 const pos = glsl::ToVec2(ConvertToLocal(m_buildingOutline.m_vertices[i],
                                                         m_params.m_tileCenter, kShapeCoordScalar));
      vertices.emplace_back(gpu::AreaVertex(glsl::vec3(pos, m_params.m_depth), ouv));
    }

    dp::GLState state(gpu::AREA_OUTLINE_PROGRAM, dp::GLState::GeometryLayer);
    state.SetColorTexture(texture);
    state.SetDrawAsLine(true);

    dp::AttributeProvider outlineProvider(1, vertices.size());
    outlineProvider.InitStream(0, gpu::AreaVertex::GetBindingInfo(), make_ref(vertices.data()));
    batcher->InsertLineRaw(state, make_ref(&outlineProvider), m_buildingOutline.m_indices);
  }
}

void AreaShape::DrawArea3D(ref_ptr<dp::Batcher> batcher, m2::PointD const & colorUv, m2::PointD const & outlineUv,
                           ref_ptr<dp::Texture> texture) const
{
  ASSERT(!m_buildingOutline.m_indices.empty(), ());
  ASSERT(!m_buildingOutline.m_normals.empty(), ());

  glsl::vec2 const uv = glsl::ToVec2(colorUv);

  vector<gpu::Area3dVertex> vertexes;
  vertexes.reserve(m_vertexes.size() + m_buildingOutline.m_normals.size() * 6);

  for (size_t i = 0; i < m_buildingOutline.m_normals.size(); i++)
  {
    size_t const startIndex = m_buildingOutline.m_indices[i * 2];
    size_t const endIndex = m_buildingOutline.m_indices[i * 2 + 1];

    glsl::vec2 const startPt = glsl::ToVec2(ConvertToLocal(m_buildingOutline.m_vertices[startIndex],
                                                           m_params.m_tileCenter, kShapeCoordScalar));
    glsl::vec2 const endPt = glsl::ToVec2(ConvertToLocal(m_buildingOutline.m_vertices[endIndex],
                                                         m_params.m_tileCenter, kShapeCoordScalar));

    glsl::vec3 normal(glsl::ToVec2(m_buildingOutline.m_normals[i]), 0.0f);
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(startPt, -m_params.m_minPosZ), normal, uv));
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(endPt, -m_params.m_minPosZ), normal, uv));
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(startPt, -m_params.m_posZ), normal, uv));

    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(startPt, -m_params.m_posZ), normal, uv));
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(endPt, -m_params.m_minPosZ), normal, uv));
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(endPt, -m_params.m_posZ), normal, uv));
  }

  glsl::vec3 normal(0.0f, 0.0f, -1.0f);
  for (auto const & vertex : m_vertexes)
  {
    glsl::vec2 const pt = glsl::ToVec2(ConvertToLocal(vertex, m_params.m_tileCenter, kShapeCoordScalar));
    vertexes.emplace_back(gpu::Area3dVertex(glsl::vec3(pt, -m_params.m_posZ), normal, uv));
  }

  dp::GLState state(gpu::AREA_3D_PROGRAM, dp::GLState::GeometryLayer);
  state.SetColorTexture(texture);
  state.SetBlending(dp::Blending(false /* isEnabled */));

  dp::AttributeProvider provider(1, vertexes.size());
  provider.InitStream(0, gpu::Area3dVertex::GetBindingInfo(), make_ref(vertexes.data()));
  batcher->InsertTriangleList(state, make_ref(&provider));

  // Generate outline.
  glsl::vec2 const ouv = glsl::ToVec2(outlineUv);

  dp::GLState outlineState(gpu::AREA_3D_OUTLINE_PROGRAM, dp::GLState::GeometryLayer);
  outlineState.SetColorTexture(texture);
  outlineState.SetBlending(dp::Blending(false /* isEnabled */));
  outlineState.SetDrawAsLine(true);

  vector<gpu::AreaVertex> vertices;
  vertices.reserve(m_buildingOutline.m_vertices.size());
  for (size_t i = 0; i < m_buildingOutline.m_vertices.size(); i++)
  {
    glsl::vec2 const pos = glsl::ToVec2(ConvertToLocal(m_buildingOutline.m_vertices[i],
                                                       m_params.m_tileCenter, kShapeCoordScalar));
    vertices.emplace_back(gpu::AreaVertex(glsl::vec3(pos, -m_params.m_posZ), ouv));
  }

  dp::AttributeProvider outlineProvider(1, vertices.size());
  outlineProvider.InitStream(0, gpu::AreaVertex::GetBindingInfo(), make_ref(vertices.data()));
  batcher->InsertLineRaw(outlineState, make_ref(&outlineProvider), m_buildingOutline.m_indices);
}

} // namespace df
