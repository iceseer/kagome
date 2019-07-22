/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_NETWORK_INJECTOR_HPP
#define KAGOME_NETWORK_INJECTOR_HPP

#include <boost/di.hpp>

// implementations
#include "libp2p/crypto/key_generator/key_generator_impl.hpp"
#include "libp2p/crypto/marshaller/key_marshaller_impl.hpp"
#include "libp2p/crypto/random_generator/boost_generator.hpp"
#include "libp2p/muxer/yamux.hpp"
#include "libp2p/network/impl/connection_manager_impl.hpp"
#include "libp2p/network/impl/dialer_impl.hpp"
#include "libp2p/network/impl/listener_manager_impl.hpp"
#include "libp2p/network/impl/network_impl.hpp"
#include "libp2p/network/impl/router_impl.hpp"
#include "libp2p/network/impl/transport_manager_impl.hpp"
#include "libp2p/peer/impl/identity_manager_impl.hpp"
#include "libp2p/protocol_muxer/multiselect.hpp"
#include "libp2p/security/plaintext.hpp"
#include "libp2p/transport/impl/upgrader_impl.hpp"
#include "libp2p/transport/tcp.hpp"

// clang-format off
/**
 * @file network_injector.hpp
 * @brief This header defines DI injector helpers, which can be used instead of
 * manual wiring.
 *
 * The main function in this header is
 * @code makeNetworkInjector() @endcode
 * Use it to create a Boost.DI container with default types.
 *
 * By default:
 * - TCP is used as transport
 * - Plaintext as security
 * - Yamux as muxer
 * - Random keypair is generated
 *
 * List of libraries that should be linked to your lib/exe:
 *  - libp2p_network
 *  - libp2p_tcp
 *  - libp2p_yamux
 *  - libp2p_plaintext
 *  - libp2p_connection_manager
 *  - libp2p_transport_manager
 *  - libp2p_listener_manager
 *  - libp2p_identity_manager
 *  - libp2p_dialer
 *  - libp2p_router
 *  - multiselect
 *  - random_generator
 *  - key_generator
 *  - marshaller
 *
 * <b>Example 1</b>: Make default network with Yamux as muxer, Plaintext as
 * security, TCP as transport.
 * @code
 * auto injector = makeNetworkInjector();
 * std::shared_ptr<Network> network = injector.create<std::shared_ptr<Network>>();
 * assert(network != nullptr);
 * @endcode
 *
 * <b>Example 2</b>: Make network with new transport, muxer and security.
 * @code
 * struct NewTransport : public TransportAdaptor {...};
 * struct NewMuxer : public MuxerAdaptor {...};
 * struct NewSecurity : public SecurityAdaptor {...};
 *
 * auto injector = makeNetworkInjector(
 *   useTransportAdaptors<NewTransport>(),
 *   useMuxerAdaptors<NewMuxer>(),
 *   useSecurityAdaptors<NewSecurity>()
 * );
 *
 * std::shared_ptr<Network> network = injector.create<std::shared_ptr<Network>>();
 * assert(network != nullptr);
 * @endcode
 *
 * <b>Example 3</b>: Use mocked router:
 * @code
 * struct RouterMock : public Router {...};
 *
 * auto injector = makeNetworkInjector(
 *   boost::di::bind<Router>.to<RouterMock>()
 * );
 *
 * // build network
 * std::shared_ptr<Network> network = injector.create<std::shared_ptr<Network>>();
 * assert(network != nullptr);
 *
 * // get mock
 * std::shared_ptr<RouterMock> routerMock = injector.create<std::shared_ptr<RouterMock>>();
 * assert(routerMock != nullptr);
 * @endcode
 *
 * <b>Example 4</b>: Use instance of mock.
 * @code
 * struct RouterMock : public Router {...};
 *
 * auto routerMock = std::make_shared<RouterMock>();
 *
 * auto injector = makeNetworkInjector(
 *   boost::di::bind<Router>.to(routerMock)
 * );
 *
 * // build network
 * std::shared_ptr<Network> network = injector.create<std::shared_ptr<Network>>();
 * assert(network != nullptr);
 * @endcode
 */

// clang-format on

namespace libp2p::injector {

  /**
   * @brief Instruct injector to use this keypair. Can be used once.
   *
   * @code
   * KeyPair keyPair = {...};
   * auto injector = makeNetworkInjector(
   *   useKeyPair(std::move(keyPair))
   * );
   * @endcode
   */
  inline auto useKeyPair(crypto::KeyPair keyPair) {
    return boost::di::bind<crypto::KeyPair>().template to(
        std::move(keyPair))[boost::di::override];
  }

  /**
   * @brief Instruct injector to use specific config type. Can be used many
   * times for different types.
   * @tparam C config type
   * @param c config instance
   * @return injector binding
   *
   * @code
   * // config definition
   * struct YamuxConfig {
   *   int a = 5;
   * }
   *
   * // config consumer definition
   * struct Yamux {
   *   Yamux(YamuxConfig config);
   * }
   *
   * // create injector
   * auto injector = makeNetworkInjector(
   *   // change default value a=5 to a=3
   *   useConfig<YamuxConfig>({.a = 3})
   * );
   * @endcode
   */
  template <typename C>
  auto useConfig(C &&c) {
    return boost::di::bind<std::decay<C>>().template to(std::forward<C>(c));
  }

  /**
   * @brief Bind security adaptors by type. Can be used once. Technically many
   * types can be specified, even the same type, but in the end only 1 instance
   * for each type is created.
   * @tparam SecImpl one or many types of security adaptors to be used
   * @return injector binding
   *
   * @code
   * struct SomeNewAdaptor : public SecurityAdaptor {...};
   *
   * auto injector = makeNetworkInjector(
   *   useSecurityAdaptors<Plaintext, SomeNewAdaptor, SecioAdaptor>()
   * );
   * @endcode
   */
  template <typename... SecImpl>
  auto useSecurityAdaptors() {
    return boost::di::bind<security::SecurityAdaptor *[]>()
        .template to<SecImpl...>()[boost::di::override];
  }

  /**
   * @brief Bind muxer adaptors by types. Can be used once. Technically many
   * types can be specified, even the same type, but in the end only 1 instance
   * for each type is created.
   * @tparam MuxerImpl one or many types of muxer adaptors to be used
   * @return injector binding
   */
  template <typename... MuxerImpl>
  auto useMuxerAdaptors() {
    return boost::di::bind<muxer::MuxerAdaptor *[]>()
        .template to<MuxerImpl...>()[boost::di::override];
  }

  /**
   * @brief Instruct injector to use these transports. Can be used once.
   * Technically many types can be specified, even the same type, but in the end
   * only 1 instance for each type is created.
   * @tparam TransportImpl one or many types of transport adaptors to be used
   * @return injector binding
   */
  template <typename... TransportImpl>
  auto useTransportAdaptors() {
    return boost::di::bind<transport::TransportAdaptor *[]>()
        .template to<TransportImpl...>()[boost::di::override];
  }

  /**
   * @brief Main function that creates Network Injector.
   * @tparam Ts types of injector bindings
   * @param args injector bindings that override default bindings.
   * @return complete network injector
   */
  template <typename... Ts>
  auto makeNetworkInjector(Ts &&... args) {
    using namespace boost;  // NOLINT

    auto csprng = std::make_shared<crypto::random::BoostRandomGenerator>();
    auto gen = std::make_shared<crypto::KeyGeneratorImpl>(*csprng);

    // assume no error here. otherwise... just blow up executable
    auto keypair = gen->generateKeys(crypto::Key::Type::ED25519).value();

    // clang-format off
    return di::make_injector(
        di::bind<crypto::KeyPair>().template to(std::move(keypair)),
        di::bind<crypto::random::CSPRNG>().template to(std::move(csprng)),
        di::bind<crypto::KeyGenerator>().template to(std::move(gen)),
        di::bind<crypto::marshaller::KeyMarshaller>().template to<crypto::marshaller::KeyMarshallerImpl>(),
        di::bind<peer::IdentityManager>().template to<peer::IdentityManagerImpl>(),

        // internal
        di::bind<network::Router>().template to<network::RouterImpl>(),
        di::bind<network::ConnectionManager>().template to<network::ConnectionManagerImpl>(),
        di::bind<network::ListenerManager>().template to<network::ListenerManagerImpl>(),
        di::bind<network::Dialer>().template to<network::DialerImpl>(),
        di::bind<network::Network>().template to<network::NetworkImpl>(),
        di::bind<network::TransportManager>().template to<network::TransportManagerImpl>(),
        di::bind<transport::Upgrader>().template to<transport::UpgraderImpl>(),
        di::bind<protocol_muxer::ProtocolMuxer>().template to<protocol_muxer::Multiselect>(),

        // default adaptors
        di::bind<security::SecurityAdaptor *[]>().template to<security::Plaintext>(),
        di::bind<muxer::MuxerAdaptor *[]>().template to<muxer::Yamux>(),
        di::bind<transport::TransportAdaptor *[]>().template to<transport::TcpTransport>(),

        // user-defined overrides...
        std::forward<decltype(args)>(args)...
    );
    // clang-format on
  }

}  // namespace libp2p::injector

#endif  // KAGOME_NETWORK_INJECTOR_HPP