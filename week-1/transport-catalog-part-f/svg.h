#pragma once

#include <cstdint>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

namespace Svg {

namespace {

constexpr auto kSvgHeader =
    std::string_view(R"(<?xml version="1.0" encoding="UTF-8" ?>)");

class PropertiesBuilder {
 public:
  explicit PropertiesBuilder(std::ostream& ostream) : ostream_(ostream) {}

  template <typename T>
  PropertiesBuilder& Add(const std::string& name, const T& value) {
    ostream_ << name << R"(=")" << value << R"(" )";
    return *this;
  }

  template <typename T>
  PropertiesBuilder& AddOptional(const std::string& name,
                                 const std::optional<T>& value) {
    if (value.has_value()) {
      return Add(name, value.value());
    }
    return *this;
  }

 private:
  std::ostream& ostream_;
};

template <typename T>
class ContentBuilder {
 public:
  explicit ContentBuilder(std::ostream& ostream) : ostream_(ostream) {}

  ContentBuilder& Add(const T& value) {
    ostream_ << value;
    return *this;
  }

 private:
  std::ostream& ostream_;
};

}  // namespace

/*
 * Point — структура из двух полей x и y типа double.
 * Необходимо иметь возможность создать точку с помощью выражения Point{x, y}, а
 * также создать с помощью конструктора по умолчанию и затем заполнить поля x и
 * y прямым обращением к ним.
 */
struct Point {
  double x, y;
};

/*
 * Rgb — структура из целочисленных полей red, green, blue.
 * Необходимо иметь возможность создать объект с помощью выражения Rgb{red,
 * green, blue}, а также создать с помощью конструктора по умолчанию и затем
 * заполнить поля red, green и blue прямым обращением к ним. Поля будут
 * заполняться значениями от 0 до 255. Валидация их инициализированности и
 * попадания значений в диапазон [0, 255] не требуется: в случае нарушения этих
 * требований допускается undefined behaviour.
 */
struct Rgb {
  int red, green, blue;
};

/*
 * Color — тип, который можно проинициализировать одним из трёх способов:
 * - Конструктором по умолчанию. Такой цвет выводится как none.
 * - Строкой (std::string). Такой цвет выводится непосредственно как содержимое
 * строки.
 * - Структурой Rgb. Такой цвет выводится в виде rgb(red,green,blue) (см.
 * примеры).
 *
 * Тип должен допускать неявную инициализацию строкой или Rgb.
 *
 * Кроме того, для удобства и улучшения читаемости должна существовать
 * глобальная константа Svg::NoneColor, представляющая собой объект класса
 * Color, созданный с помощью конструктора по умолчанию.
 */
class Color {
 public:
  Color(const std::string& color) : color_(color) {}
  Color(std::string&& color) : color_(std::move(color)) {}
  Color() : color_("none") {}

  template <typename TRgb>
  Color(TRgb&& color) : color_(std::forward<TRgb>(color)) {}

  [[nodiscard]] std::string ToString() const {
    return std::visit(ToStringVisitor{}, color_);
  }

 private:
  class ToStringVisitor {
   public:
    std::string operator()(const std::string& color) { return color; }
    std::string operator()(const Rgb& rgb) {
      std::stringstream s{};
      s << "rgb(" << rgb.red << "," << rgb.green << "," << rgb.blue << ")";
      return std::string{s.str()};
    }
  };

 private:
  std::variant<std::string, Rgb> color_;
};

const Color NoneColor;

class Tag {
 public:
  explicit Tag(std::string tag) : tag_(std::move(tag)) {}
  virtual ~Tag() = default;

  [[nodiscard]] const std::string& GetTag() const { return tag_; }

  virtual void Render(std::ostream& ostream) const {
    RenderOpenTag(ostream);
    RenderProperties(ostream);
    RenderCloseTag(ostream);
  }

 protected:
  virtual void RenderOpenTag(std::ostream& ostream) const {
    ostream << '<' << tag_ << ' ';
  }

  virtual void RenderProperties(std::ostream& ostream) const {
    PropertiesBuilder properties{ostream};
    AddProperties(properties);
  }

  virtual void RenderCloseTag(std::ostream& ostream) const { ostream << "/>"; }

  virtual void AddProperties(PropertiesBuilder& /*properties*/) const {}

 private:
  std::string tag_;
};

template <typename T>
class TagWithContent : public Tag {
 public:
  explicit TagWithContent(const std::string& tag) : Tag(tag) {}
  explicit TagWithContent(std::string&& tag) : Tag(tag) {}

  void Render(std::ostream& ostream) const override {
    RenderOpenTag(ostream);
    RenderProperties(ostream);
    RenderContent(ostream);
    RenderCloseTag(ostream);
  }

 protected:
  virtual void RenderContent(std::ostream& ostream) const {
    ostream << '>';
    ContentBuilder<T> builder{ostream};
    AddContent(builder);
  }

  void RenderCloseTag(std::ostream& ostream) const override {
    ostream << "</" << GetTag() << '>';
  }

  virtual void AddContent(ContentBuilder<T>& builder) const = 0;
};

namespace {

template <typename T>
class BaseProperties {
 public:
  // Задаёт значение свойства fill — цвет заливки. Значение по умолчанию —
  // NoneColor.
  T& SetFillColor(const Color& fill_color) {
    fill_color_ = fill_color;
    return ThisRef();
  }
  // задаёт значение свойства stroke — цвет линии. Значение по умолчанию —
  // NoneColor.
  T& SetStrokeColor(const Color& stroke_color) {
    stroke_color_ = stroke_color;
    return ThisRef();
  }
  // задаёт значение свойства stroke-width — толщину линии. Значение по
  // умолчанию — 1.0.
  T& SetStrokeWidth(double stroke_width) {
    stroke_width_ = stroke_width;
    return ThisRef();
  }
  // задаёт значение свойства stroke-linecap — тип формы конца линии. По
  // умолчанию свойство не выводится.
  T& SetStrokeLineCap(const std::string& stroke_line_cap) {
    stroke_line_cap_ = stroke_line_cap;
    return ThisRef();
  }
  // задаёт значение свойства stroke-linejoin — тип формы соединения линий. По
  // умолчанию свойство не выводится.
  T& SetStrokeLineJoin(const std::string& stroke_line_join) {
    stroke_line_join_ = stroke_line_join;
    return ThisRef();
  }

 protected:
  void AddProperties(PropertiesBuilder& builder) const {
    builder.Add("fill", fill_color_.ToString())
        .Add("stroke", stroke_color_.ToString())
        .Add("stroke-width", stroke_width_)
        .AddOptional("stroke-linecap", stroke_line_cap_)
        .AddOptional("stroke-linejoin", stroke_line_join_);
  }

 private:
  T& ThisRef() { return static_cast<T&>(*this); }

 private:
  Color fill_color_ = NoneColor;
  Color stroke_color_ = NoneColor;
  double stroke_width_ = 1.0;
  std::optional<std::string> stroke_line_cap_ = std::nullopt;
  std::optional<std::string> stroke_line_join_ = std::nullopt;
};

}  // namespace

/*
 * SetCenter(Point): задаёт значения свойств cx и cy — координаты центра круга.
 * Значения по умолчанию — 0.0. SetRadius(double): задаёт значение свойства r —
 * радиус круга. Значение по умолчанию — 1.0.
 */
class Circle : public Tag, public BaseProperties<Circle> {
 public:
  Circle() : Tag("circle") {}

  Circle& SetCenter(Point point) {
    center_ = point;
    return *this;
  }

  Circle& SetRadius(double radius) {
    radius_ = radius;
    return *this;
  }

 protected:
  void AddProperties(PropertiesBuilder& builder) const override {
    BaseProperties::AddProperties(builder);
    builder.Add("cx", center_.x).Add("cy", center_.y).Add("r", radius_);
  }

 private:
  Point center_ = {.x = 0, .y = 0};
  double radius_ = 1.0;
};

/*
 * Polyline
 * AddPoint(Point): добавляет вершину ломаной — элемент свойства points,
 * записываемый в виде x,y и отделяемый пробелами от соседних элементов (см.
 * примеры). Значение свойства по умолчанию — пустая строка.
 */
class Polyline : public Tag, public BaseProperties<Polyline> {
 public:
  Polyline() : Tag("polyline") {}

  Polyline& AddPoint(const Point& point) {
    if (!points_.empty()) {
      points_.push_back(' ');
    }
    std::stringstream s{};
    s << point.x << "," << point.y;
    points_.append(s.str());
    return *this;
  }

 protected:
  void AddProperties(PropertiesBuilder& builder) const override {
    BaseProperties::AddProperties(builder);
    builder.Add("points", points_);
  }

 private:
  std::string points_{};
};

class Text : public TagWithContent<std::string>, public BaseProperties<Text> {
 public:
  Text() : TagWithContent("text") {}

  // задаёт значения свойств x и y — координаты текста. Значения по умолчанию —
  // 0.0.
  Text& SetPoint(Point point) {
    point_ = point;
    return *this;
  }

  // задаёт значения свойств dx и dy — величины отступа текста от координаты.
  // Значения по умолчанию — 0.0.
  Text& SetOffset(Point offset) {
    offset_ = offset;
    return *this;
  }

  // задаёт значение свойства font-size — размер шрифта. Значение по умолчанию
  // — 1.
  Text& SetFontSize(uint32_t font_size) {
    font_size_ = font_size;
    return *this;
  }

  // задаёт значение свойства font-family — название шрифта. По умолчанию
  // свойство не выводится.
  Text& SetFontFamily(const std::string& font_family) {
    font_family_ = font_family;
    return *this;
  }

  // задаёт содержимое тега <text> — непосредственно выводимый текст. По
  // умолчанию текст пуст.
  Text& SetData(const std::string& data) {
    data_ = data;
    return *this;
  }

 protected:
  void AddProperties(PropertiesBuilder& builder) const override {
    BaseProperties::AddProperties(builder);
    builder.Add("x", point_.x)
        .Add("y", point_.y)
        .Add("dx", offset_.x)
        .Add("dy", offset_.y)
        .Add("font-size", font_size_)
        .AddOptional("font-family", font_family_);
  }

  void AddContent(ContentBuilder<std::string>& builder) const override {
    builder.Add(data_);
  }

 private:
  Point point_ = {.x = 0, .y = 0};
  Point offset_ = {.x = 0, .y = 0};
  uint32_t font_size_ = 1;
  std::optional<std::string> font_family_ = std::nullopt;
  std::string data_ = "";
};

/*
 * Document — класс, с помощью которого производится компоновка и отрисовка
 * SVG-документа. Класс должен поддерживать следующие операции:
 *
 * Создание с помощью конструктора по умолчанию: Svg::Document svg;
 * Добавление объекта: svg.Add(object), где object имеет тип Circle, Polyline
 * или Text. Обратите внимание, что таким образом поддерживается лишь линейная
 * структура документа: составляющие его объекты по сути образуют массив.
 * Отрисовка (формирование результирующей строки): svg.Render(out), где out —
 * наследник std::ostream.
 */
class Document {
 public:
  Document() : stream_() {}

  void Add(const Tag& tag) { tag.Render(stream_); }

  void Render(std::ostream& ostream) const {
    ostream << kSvgHeader;
    ostream << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
    ostream << stream_.str();
    ostream << "</svg>";
  }

 private:
  std::stringstream stream_;
};

}  // namespace Svg
