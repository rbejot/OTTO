#include "testing.t.hpp"

#include <functional>

#include "core/props/props.hpp"
#include "core/props/mixins/all.hpp"

namespace otto::core::props {

  TEST_CASE("Propeties", "[props]")
  {
    SECTION("Basic mixin tests")
    {
      // Test the tags of a property

      Property<float, mixins::steppable, mixins::has_name> pf;

      OTTO_META_ASSERT_EQUAL(
        decltype(pf)::tag_list,
        meta::_t<normalize_tags<
        tag_list<mixins::steppable, mixins::has_value, mixins::has_name>>>);

      // Assert that the mixin implementation is inherited by the property
      static_assert(
        std::is_base_of_v<MixinTag::mixin_t<mixins::steppable, float,
                                            make_tag_list_t<mixins::steppable,
                                                            mixins::has_name>>,
                          decltype(pf)>);

      pf.set(3.f);
      REQUIRE(pf.get() == 3.f);
      pf.init<mixins::steppable>(2.f);
      pf.init<mixins::has_name>("property");
      pf.step(1);
      REQUIRE(pf == 5.f);
      float f = pf;
    }

    struct Props {
      Property<float,
               mixins::steppable,
               mixins::has_limits,
               mixins::has_name,
               mixins::faust_link>
        pf1 = 0.f;
      Property<float, mixins::steppable, mixins::has_name, mixins::faust_link>
        pf2 = 1.f;

      Props()
      {
        pf1.init<mixins::steppable>(1.f).init<mixins::has_limits>(-5.f, 5.f);
      };
    } props;

    CONCEPT_ASSERT(cpts::models<HookTag, mixins::has_value::hooks::on_set,
                                typename decltype(props.pf1)::value_type>());

    static_assert(MixinImpl::has_handler_v<
                  typename decltype(props.pf1)::mixin<mixins::has_limits>,
                  mixins::has_value::hooks::on_set, HookOrder::Early>);

    // REQUIRE(props.pf1 == 0.f);
    REQUIRE(props.pf2 == 1.f);
    props.pf1 = 10.f;
    // Test limits
    REQUIRE(props.pf1 == 5.f);
  }
} // namespace otto::core::props

namespace otto::core::props {

  using namespace mixins;

  Property<float, has_value, steppable, has_limits, serializable> property = 0.f;

  struct Props : Properties<serializable> {
    Property<float, has_value, serializable> prop1 = {
      has_value::init(0),
      has_name::init("prop1")
    };
    Property<float, has_value, serializable> prop2 = {
      has_name::init("prop2"),
      has_value::init(4)
    };

    Props() : Properties(prop1, prop2) {}
  } props;


  TEST_CASE("Property conversions", "[props]") {
    auto& pb = static_cast<PropertyBase&>(property);
    REQUIRE(pb == property);

    const float& val = static_cast<const float&>(property);
    REQUIRE(&val == &property.get());
  }

  TEST_CASE("mixins::serializable", "[props]") {
    REQUIRE(props.prop1.to_json() == 0.f);
    REQUIRE(props.prop2.to_json() == 4.f);
    auto expct = nlohmann::json{{"prop1", 0.f}, {"prop2", 4.f}};
    LOGI("102");
    auto& interface = props.interface<serializable>();
    REQUIRE(interface.name() == "Test");
    auto got = interface.to_json();
    LOGI("106");
    REQUIRE(got == expct);
  }

  TEST_CASE("mixins::has_limits", "[props]") {
    // Alternative, nicer initialization syntax. tag::init returns a type that
    // contains the args forwarded by std::forward_tuple. This would enable us
    // (with more magic metaprogramming) to actually use constructors for the
    // mixins.

    Property<float, has_value, steppable, has_limits, serializable> pp = {
      has_limits::init(1, 5), //
      steppable::init(1.5f),  //
    };

    REQUIRE(pp.min == 1.f);
    REQUIRE(pp.max == 5.f);
    REQUIRE(pp.step_size == 1.5f);

    pp.set(4);
    REQUIRE(pp == 4.f);
    pp.set(5);
    REQUIRE(pp == 5.f);
    pp = 5.0001f;
    REQUIRE(pp == 5.f);
    pp = 1023095.f;
    REQUIRE(pp == 5.f);
    pp = 1.f;
    REQUIRE(pp == 1.f);
    pp = 0.f;
    REQUIRE(pp == 1.f);
    pp = -203.f;
    REQUIRE(pp == 1.f);
    pp.min = -10;
    REQUIRE(pp == 1.f);
    pp = -3;
    REQUIRE(pp == -3.f);
    pp = -30;
    REQUIRE(pp == -10.f);
  }
}
