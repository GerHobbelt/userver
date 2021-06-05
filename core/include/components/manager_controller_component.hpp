#pragma once

/// @file components/manager_controller_component.hpp
/// @brief @copybrief components::ManagerControllerComponent

#include <components/component_config.hpp>
#include <components/component_context.hpp>
#include <components/impl/component_base.hpp>
#include <engine/task/task_processor_fwd.hpp>
#include <utils/statistics/storage.hpp>

#include <taxi_config/storage/component.hpp>

namespace components {

class Manager;

// clang-format off

/// @defgroup userver_components Userver Components
///
/// @brief Components that could be added to components::ComponentList for
/// further use with utils::DaemonMain, components::Run or components::RunOnce.
///
/// Any userver-based service consists of components. A component is a basic
/// building block that encapsulates dependencies logic with configuration and
/// is able to interact with other components.
///
/// Only components should know about components. Clients and other types
/// constructed by components should not use components::ComponentConfig,
/// components::ComponentContext, or components directly. All the components
/// should inherit from components::LoggableComponentBase base class and may
/// override its methods.
///
/// ## Components static configuration
/// components::ManagerControllerComponent starts all the components that
/// were added to the components::ComponentList. Each registered component
/// should have a section in service config (also known as static config).
///
/// The component configuration is passed as a first parameter of type
/// components::ComponentConfig to the constructor of the component. Note that
/// components::ComponentConfig extends the functionality of
/// yaml_config::YamlConfig that is able to substitute variables with values,
/// use fallbacks. See yaml_config::YamlConfig for more info and examples.
///
/// All the components have the following options:
///
/// Name | Description | Default value
/// ---- | ----------- | -------------
/// load-enabled | set to `false` to disable loading of the component | true
///
/// ## Startup context
/// On component construction a components::ComponentContext is passed as a
/// second parameter to the constructor of the component. That context could
/// be used to get references to other components. That reference to the
/// component is guaranteed to outlive the component that is being constructed.
///
/// ## Components construction order
/// utils::DaemonMain, components::Run or components::RunOnce
/// start all the components from the passed components::ComponentList.
/// Each component is constructed in a separate engine::Task which makes them
/// initialize concurrently. This is a useful feature, for examples in cases
/// with multiple caches that slowly read from different databases.
///
/// To make component *A* depend on component *B* just call
/// components::ComponentContext::FindComponent<B>() in the constructor of A.
/// FindComponent() suspends the current task and continues only after the
/// construction of component B is finished. Components are destroyed
/// in reverse order of construction, so the component A is destroyed before
/// the component B. In other words - references from FindComponent() outlive
/// the component that called the FindComponent() function. If any component
/// loading fails, FindComponent() wakes up and throws an
/// components::ComponentsLoadCancelledException.
///
/// ## References from components
/// It is a common practice to have a component that returns reference *R* from
/// some function *F*. In such cases:
/// * a reference *R* lives as long as the component is alive
/// * and it should be safe to invoke member function of reference *R*
/// concurrently with member function invocation on a result of other F
/// invocation.
///
/// Examples:
/// * components::HttpClient::GetHttpClient()
/// * components::StatisticsStorage::GetStorage()
///
/// ## Writing your own components
/// Users of the framework may (and should) write their own components.
///
/// Components provide functionality to tie the main part of the program with
/// the configuration and other components. Component should be lightweight
/// and simple.
///
/// @note Rule of a thumb: if you wish to unit test some code that is located
/// in the component, then in 99% of cases that code should not be located in
/// the component.
///
/// ### Should I write a new component or class would be enough?
/// You need a component if:
/// * you need a static config
/// * you need to work with other components
/// * you are writing clients (you need a component to be the factory for your
/// clients)
/// * you want to subscribe for configs or cache changes
///
/// ### HowTo
/// Start writing your component from adding a header file with a class
/// inherited from components::LoggableComponentBase.
/// @snippet components/component_sample_test.hpp  Sample user component header
///
/// In source file write the implementation of the componenet:
/// @snippet components/component_sample_test.cpp  Sample user component source
/// Destructor of the component is invoked on service shutdown. Components are
/// destroyed in the reverse order of construction. In other words
/// - references from context.FindComponent<TaxiConfig>() outlive the
/// component.
///
/// If you need runtime configs, you can get them using this approach:
/// @snippet components/component_sample_test.cpp  Sample user component runtime config source
///
/// @note See @ref md_en_userver_tutorial_config_service for info on how to
/// implement your own config server.
///
/// Do not forget to register your component in components::ComponentList
/// before invoking the utils::DaemonMain, components::Run or
/// components::RunOnce.
///
/// Done! You've implemented your first component. Full sources:
/// * @ref components/component_sample_test.hpp
/// * @ref components/component_sample_test.cpp
///
/// @note For info on writing HTTP handler components refer to
/// the @ref md_en_userver_tutorial_hello_service.
///
/// ### Testing
/// Starting up the components in unit tests is quite hard. Prefer moving out
/// all the functionality from the component or testing the component with the
/// help of testsuite https://github.com/yandex/yandex-taxi-testsuite .
///
/// @example components/component_sample_test.hpp
/// @example components/component_sample_test.cpp


/// @ingroup userver_components
///
/// @brief Component to start all the other components
///
/// ## Dynamic config
/// * @ref USERVER_TASK_PROCESSOR_PROFILER_DEBUG
/// * @ref USERVER_TASK_PROCESSOR_QOS
///
/// ## Static options:
/// Name | Description | Default value
/// ---- | ----------- | -------------
/// coro_pool.initial_size | amount of coroutines to preallocate on startup | -
/// coro_pool.max_size | max amount of coroutines to keep preallocated | -
/// event_thread_pool.threads | number of threads to process low level IO system calls (number of ev loops to start in libev) | -
/// components | dictionary of "component name": "options" | -
/// task_processors | dictionary of task processors and their options | -
/// task_processors.*NAME*.thread_name | set OS thread name to this value | -
/// task_processors.*NAME*.worker_threads | threads count for the task processor | -
/// default_task_processor | name of the default task processor to use in components | -
///
/// ## Static configuration example:
///
/// @snippet components/common_component_list_test.cpp  Sample components manager config component config

// clang-format on
class ManagerControllerComponent final : public impl::ComponentBase {
 public:
  ManagerControllerComponent(const components::ComponentConfig& config,
                             const components::ComponentContext& context);

  ~ManagerControllerComponent() override;

  static constexpr const char* kName = "manager-controller";

 private:
  formats::json::Value ExtendStatistics(
      const utils::statistics::StatisticsRequest& /*request*/);

  using TaxiConfigPtr = std::shared_ptr<const taxi_config::Config>;
  void OnConfigUpdate(const TaxiConfigPtr& cfg);

 private:
  const components::Manager& components_manager_;
  utils::statistics::Entry statistics_holder_;
  utils::AsyncEventSubscriberScope config_subscription_;
};

}  // namespace components
