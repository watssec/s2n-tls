/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "s2n.h"
#include "s2n_test.h"

#include "stuffer/s2n_stuffer.h"
#include "testlib/s2n_testlib.h"

#include "tls/extensions/s2n_client_renegotiation_info.h"
#include "tls/extensions/s2n_cookie.h"
#include "tls/extensions/s2n_extension_type_lists.h"
#include "tls/extensions/s2n_server_supported_versions.h"
#include "tls/s2n_cipher_suites.h"
#include "tls/s2n_security_policies.h"
#include "tls/s2n_tls.h"
#include "tls/s2n_tls13.h"
#include "tls/s2n_tls13_handshake.h"
#include "tls/s2n_connection.h"
#include "pq-crypto/s2n_pq.h"

/* This include is required to access static function s2n_server_hello_parse */
#include "tls/s2n_server_hello.c"

#include "error/s2n_errno.h"
#include "utils/s2n_safety.h"

#define HELLO_RETRY_MSG_NO 1
#define SERVER_HELLO_MSG_NO 5

int main(int argc, char **argv)
{
    BEGIN_TEST();

    if (!s2n_is_tls13_fully_supported()) {
        END_TEST();
    }

    EXPECT_SUCCESS(s2n_enable_tls13_in_test());

    /* Test s2n_server_hello_retry_recv */
    {
        /* s2n_server_hello_retry_recv must fail when a keyshare for a matching curve was already present */
        {
            struct s2n_config *config;
            struct s2n_connection *conn;

            EXPECT_NOT_NULL(config = s2n_config_new());
            EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
            EXPECT_SUCCESS(s2n_connection_set_config(conn, config));

            const struct s2n_ecc_preferences *ecc_pref = NULL;
            POSIX_GUARD(s2n_connection_get_ecc_preferences(conn, &ecc_pref));
            EXPECT_NOT_NULL(ecc_pref);

            conn->actual_protocol_version = S2N_TLS13;
            conn->kex_params.server_ecc_evp_params.negotiated_curve = ecc_pref->ecc_curves[0];
            conn->kex_params.client_ecc_evp_params.negotiated_curve = ecc_pref->ecc_curves[0];

            EXPECT_NULL(conn->kex_params.client_ecc_evp_params.evp_pkey);
            EXPECT_SUCCESS(s2n_ecc_evp_generate_ephemeral_key(&conn->kex_params.client_ecc_evp_params));
            EXPECT_NOT_NULL(conn->kex_params.client_ecc_evp_params.evp_pkey);

            EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_INVALID_HELLO_RETRY);

            EXPECT_SUCCESS(s2n_config_free(config));
            EXPECT_SUCCESS(s2n_connection_free(conn));
        }

        /* s2n_server_hello_retry_recv must fail for a connection with actual protocol version less than TLS13 */
        {
            struct s2n_config *config;
            struct s2n_connection *conn;

            EXPECT_NOT_NULL(config = s2n_config_new());
            EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
            EXPECT_SUCCESS(s2n_connection_set_config(conn, config));

            conn->actual_protocol_version = S2N_TLS12;

            EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_INVALID_HELLO_RETRY);

            EXPECT_SUCCESS(s2n_config_free(config));
            EXPECT_SUCCESS(s2n_connection_free(conn));
        }

        /* Test ECC success case for s2n_server_hello_retry_recv */
        {
            struct s2n_config *server_config;
            struct s2n_config *client_config;

            struct s2n_connection *server_conn;
            struct s2n_connection *client_conn;

            struct s2n_cert_chain_and_key *tls13_chain_and_key;
            char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
            char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

            EXPECT_NOT_NULL(server_config = s2n_config_new());
            EXPECT_NOT_NULL(client_config = s2n_config_new());

            EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
            EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

            EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
            EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
            EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
            EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
            EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(client_config, tls13_chain_and_key));
            EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(server_config, tls13_chain_and_key));

            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, server_config));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* ClientHello 1 */
            EXPECT_SUCCESS(s2n_client_hello_send(client_conn));

            EXPECT_SUCCESS(s2n_stuffer_copy(&client_conn->handshake.io, &server_conn->handshake.io,
                                            s2n_stuffer_data_available(&client_conn->handshake.io)));

            /* Server receives ClientHello 1 */
            EXPECT_SUCCESS(s2n_client_hello_recv(server_conn));

            server_conn->kex_params.server_ecc_evp_params.negotiated_curve = s2n_all_supported_curves_list[0];
            EXPECT_SUCCESS(s2n_set_connection_hello_retry_flags(server_conn));

            /* Server sends HelloRetryMessage */
            EXPECT_SUCCESS(s2n_server_hello_retry_send(server_conn));

            EXPECT_SUCCESS(s2n_stuffer_wipe(&client_conn->handshake.io));
            EXPECT_SUCCESS(s2n_stuffer_copy(&server_conn->handshake.io, &client_conn->handshake.io,
                                            s2n_stuffer_data_available(&server_conn->handshake.io)));
            client_conn->handshake.message_number = HELLO_RETRY_MSG_NO;
            /* Read the message off the wire */
            EXPECT_SUCCESS(s2n_server_hello_parse(client_conn));
            client_conn->actual_protocol_version_established = 1;

            EXPECT_SUCCESS(s2n_conn_set_handshake_type(client_conn));
            /* Client receives the HelloRetryRequest mesage */
            EXPECT_SUCCESS(s2n_server_hello_retry_recv(client_conn));

            EXPECT_SUCCESS(s2n_config_free(client_config));
            EXPECT_SUCCESS(s2n_config_free(server_config));
            EXPECT_SUCCESS(s2n_connection_free(server_conn));
            EXPECT_SUCCESS(s2n_connection_free(client_conn));
            EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
        }

        {
            const struct s2n_kem_group *test_kem_groups[] = {
                &s2n_secp256r1_sike_p434_r3,
                &s2n_secp256r1_bike1_l1_r2,
            };

            const struct s2n_kem_preferences test_kem_prefs = {
                .kem_count = 0,
                .kems = NULL,
                .tls13_kem_group_count = s2n_array_len(test_kem_groups),
                .tls13_kem_groups = test_kem_groups,
            };

            const struct s2n_security_policy test_security_policy = {
                .minimum_protocol_version = S2N_SSLv3,
                .cipher_preferences = &cipher_preferences_test_all_tls13,
                .kem_preferences = &test_kem_prefs,
                .signature_preferences = &s2n_signature_preferences_20200207,
                .ecc_preferences = &s2n_ecc_preferences_20200310,
            };

            if (!s2n_pq_is_enabled()) {
                struct s2n_connection *conn;
                EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
                conn->actual_protocol_version = S2N_TLS13;
                conn->security_policy_override = &test_security_policy;

                const struct s2n_kem_preferences *kem_pref = NULL;
                POSIX_GUARD(s2n_connection_get_kem_preferences(conn, &kem_pref));
                EXPECT_NOT_NULL(kem_pref);

                conn->kex_params.server_kem_group_params.kem_group = kem_pref->tls13_kem_groups[0];
                EXPECT_NULL(conn->kex_params.server_ecc_evp_params.negotiated_curve);

                EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_PQ_DISABLED);

                EXPECT_SUCCESS(s2n_connection_free(conn));
            } else {
                /* s2n_server_hello_retry_recv must fail when a keyshare for a matching PQ KEM was already present */
                {
                    struct s2n_connection *conn;
                    EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
                    conn->actual_protocol_version = S2N_TLS13;
                    conn->security_policy_override = &test_security_policy;

                    const struct s2n_kem_preferences *kem_pref = NULL;
                    POSIX_GUARD(s2n_connection_get_kem_preferences(conn, &kem_pref));
                    EXPECT_NOT_NULL(kem_pref);

                    conn->kex_params.server_kem_group_params.kem_group = kem_pref->tls13_kem_groups[0];
                    EXPECT_NULL(conn->kex_params.server_ecc_evp_params.negotiated_curve);

                    struct s2n_kem_group_params *client_params = &conn->kex_params.client_kem_group_params;
                    client_params->kem_group = kem_pref->tls13_kem_groups[0];
                    client_params->kem_params.kem = kem_pref->tls13_kem_groups[0]->kem;
                    client_params->ecc_params.negotiated_curve = kem_pref->tls13_kem_groups[0]->curve;

                    EXPECT_NULL(client_params->ecc_params.evp_pkey);
                    EXPECT_NULL(client_params->kem_params.private_key.data);

                    kem_public_key_size public_key_size = kem_pref->tls13_kem_groups[0]->kem->public_key_length;
                    EXPECT_SUCCESS(s2n_alloc(&client_params->kem_params.public_key, public_key_size));

                    EXPECT_OK(s2n_kem_generate_keypair(&client_params->kem_params));
                    EXPECT_NOT_NULL(client_params->kem_params.private_key.data);
                    EXPECT_SUCCESS(s2n_ecc_evp_generate_ephemeral_key(&client_params->ecc_params));
                    EXPECT_NOT_NULL(client_params->ecc_params.evp_pkey);

                    EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_INVALID_HELLO_RETRY);

                    EXPECT_SUCCESS(s2n_free(&client_params->kem_params.public_key));
                    EXPECT_SUCCESS(s2n_connection_free(conn));
                }
                /* Test failure if exactly one of {named_curve, kem_group} isn't non-null */
                {
                    struct s2n_connection *conn;
                    EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
                    conn->actual_protocol_version = S2N_TLS13;
                    conn->security_policy_override = &test_security_policy;

                    conn->kex_params.server_kem_group_params.kem_group = &s2n_secp256r1_sike_p434_r3;
                    conn->kex_params.server_ecc_evp_params.negotiated_curve = &s2n_ecc_curve_secp256r1;

                    EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_INVALID_HELLO_RETRY);

                    conn->kex_params.server_kem_group_params.kem_group = NULL;
                    conn->kex_params.server_ecc_evp_params.negotiated_curve = NULL;

                    EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_retry_recv(conn), S2N_ERR_INVALID_HELLO_RETRY);

                    EXPECT_SUCCESS(s2n_connection_free(conn));
                }
                /* Test PQ KEM success case for s2n_server_hello_retry_recv. */
                {
                    struct s2n_config *config;
                    struct s2n_connection *conn;

                    struct s2n_cert_chain_and_key *tls13_chain_and_key;
                    char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
                    char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

                    EXPECT_NOT_NULL(config = s2n_config_new());
                    EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
                    conn->security_policy_override = &test_security_policy;

                    EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
                    EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
                    EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
                    EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
                    EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(config, tls13_chain_and_key));

                    /* Client sends ClientHello containing key share for p256+SIKE
                     * (but indicates support for p256+BIKE in supported_groups) */
                    EXPECT_SUCCESS(s2n_client_hello_send(conn));

                    EXPECT_SUCCESS(s2n_stuffer_wipe(&conn->handshake.io));
                    conn->session_id_len = 0; /* Wipe the session id to match the HRR hex */

                    /* Server responds with HRR indicating p256+BIKE as choice for negotiation;
                     * the last 6 bytes (0033 0002 2F23) are the key share extension with p256+BIKE */
                    DEFER_CLEANUP(struct s2n_stuffer hrr = {0}, s2n_stuffer_free);
                    EXPECT_SUCCESS(s2n_stuffer_alloc_ro_from_hex_string(&hrr,
                            "0303CF21AD74E59A6111BE1D8C021E65B891C2A211167ABB8C5E079E09E2C8A8339C00130200000C002B00020304003300022F23"));

                    EXPECT_SUCCESS(s2n_stuffer_copy(&hrr, &conn->handshake.io, s2n_stuffer_data_available(&hrr)));
                    conn->handshake.message_number = HELLO_RETRY_MSG_NO;
                    /* Read the message off the wire */
                    EXPECT_SUCCESS(s2n_server_hello_parse(conn));
                    conn->actual_protocol_version_established = 1;

                    EXPECT_SUCCESS(s2n_conn_set_handshake_type(conn));
                    /* Client receives the HelloRetryRequest message */
                    EXPECT_SUCCESS(s2n_server_hello_retry_recv(conn));

                    EXPECT_SUCCESS(s2n_config_free(config));
                    EXPECT_SUCCESS(s2n_connection_free(conn));
                    EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
                }
            }
        }
    }

    /* Verify that the hash transcript recreation function is called correctly,
     * within the s2n_server_hello_retry_send and s2n_server_hello_retry_recv functions.
     * The hash transcript recreation function, if called correctly takes the existing ClientHello1
     * hash, and generates a synthetic message. This test verifies that transcript hash recreated is the same
     * on both the server and client side. */
    {
        struct s2n_config *server_config;
        struct s2n_config *client_config;

        struct s2n_connection *server_conn;
        struct s2n_connection *client_conn;

        struct s2n_cert_chain_and_key *tls13_chain_and_key;
        char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
        char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

        EXPECT_NOT_NULL(server_config = s2n_config_new());
        EXPECT_NOT_NULL(client_config = s2n_config_new());

        EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
        EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

        EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(client_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(server_config, tls13_chain_and_key));

        EXPECT_SUCCESS(s2n_connection_set_config(server_conn, server_config));

        /* Force the HRR path */
        client_conn->security_policy_override = &security_policy_test_tls13_retry;

        /* ClientHello 1 */
        EXPECT_SUCCESS(s2n_client_hello_send(client_conn));

        EXPECT_SUCCESS(s2n_stuffer_copy(&client_conn->handshake.io, &server_conn->handshake.io,
                                        s2n_stuffer_data_available(&client_conn->handshake.io)));

        /* Server receives ClientHello 1 */
        EXPECT_SUCCESS(s2n_client_hello_recv(server_conn));
        EXPECT_TRUE(s2n_is_hello_retry_handshake(server_conn));
        EXPECT_SUCCESS(s2n_set_connection_hello_retry_flags(server_conn));

        server_conn->kex_params.server_ecc_evp_params.negotiated_curve = s2n_all_supported_curves_list[0];

        /* Server sends HelloRetryRequest message, note that s2n_server_hello_retry_recreate_transcript
         * is called within the s2n_server_hello_retry_send function */
        EXPECT_SUCCESS(s2n_server_hello_retry_send(server_conn));

        s2n_tls13_connection_keys(server_keys, server_conn);
        uint8_t hash_digest_length = server_keys.size;

        /* Obtain the transcript hash recreated within the HelloRetryRequest message */
        DEFER_CLEANUP(struct s2n_hash_state server_hash = { 0 }, s2n_hash_free);
        uint8_t server_digest_out[S2N_MAX_DIGEST_LEN] = { 0 };
        POSIX_GUARD(s2n_hash_new(&server_hash));
        POSIX_GUARD_RESULT(s2n_handshake_copy_hash_state(server_conn, server_keys.hash_algorithm, &server_hash));
        POSIX_GUARD(s2n_hash_digest(&server_hash, server_digest_out, hash_digest_length));

        struct s2n_blob server_blob;
        EXPECT_SUCCESS(s2n_blob_init(&server_blob, server_digest_out, hash_digest_length));

        EXPECT_SUCCESS(s2n_stuffer_wipe(&client_conn->handshake.io));
        EXPECT_SUCCESS(s2n_stuffer_copy(&server_conn->handshake.io, &client_conn->handshake.io,
                                        s2n_stuffer_data_available(&server_conn->handshake.io)));
        client_conn->handshake.message_number = HELLO_RETRY_MSG_NO;
        /* Client receives the HelloRetryRequest mesage, note that s2n_server_hello_retry_recreate_transcript
         * is called within the s2n_server_hello_recv function */
        EXPECT_SUCCESS(s2n_server_hello_recv(client_conn));

        s2n_tls13_connection_keys(client_keys, client_conn);
        hash_digest_length = client_keys.size;

        /* Obtain the transcript hash recreated within ClientHello2 message */
        DEFER_CLEANUP(struct s2n_hash_state client_hash = { 0 }, s2n_hash_free);
        uint8_t client_digest_out[S2N_MAX_DIGEST_LEN];
        POSIX_GUARD(s2n_hash_new(&client_hash));
        POSIX_GUARD_RESULT(s2n_handshake_copy_hash_state(client_conn, client_keys.hash_algorithm, &client_hash));
        POSIX_GUARD(s2n_hash_digest(&client_hash, client_digest_out, hash_digest_length));

        struct s2n_blob client_blob;
        EXPECT_SUCCESS(s2n_blob_init(&client_blob, client_digest_out, hash_digest_length));

        /* Test that the transcript hash recreated MUST be the same on the server and client side */
        S2N_BLOB_EXPECT_EQUAL(client_blob, server_blob);

        EXPECT_SUCCESS(s2n_config_free(client_config));
        EXPECT_SUCCESS(s2n_config_free(server_config));
        EXPECT_SUCCESS(s2n_connection_free(server_conn));
        EXPECT_SUCCESS(s2n_connection_free(client_conn));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
    }

    /* Self-Talk test: the client initiates a handshake with an unknown keyshare.
     * The server sends a HelloRetryRequest that requires the client to generate a
     * key share on the server negotiated curve. */
    {
        struct s2n_config *server_config;
        struct s2n_config *client_config;

        struct s2n_connection *server_conn;
        struct s2n_connection *client_conn;

        struct s2n_cert_chain_and_key *tls13_chain_and_key;
        char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
        char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

         /* Create nonblocking pipes */
        struct s2n_test_io_pair io_pair;
        EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));

        EXPECT_NOT_NULL(server_config = s2n_config_new());
        EXPECT_NOT_NULL(client_config = s2n_config_new());

        EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
        EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

        EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(client_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(server_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_disable_x509_verification(client_config));

        EXPECT_SUCCESS(s2n_connection_set_config(server_conn, server_config));
        EXPECT_SUCCESS(s2n_connection_set_config(client_conn, client_config));

        server_conn->x509_validator.skip_cert_validation = 1;
        client_conn->x509_validator.skip_cert_validation = 1;

        /* Force the HRR path */
        client_conn->security_policy_override = &security_policy_test_tls13_retry;

        EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

        /* Negotiate handshake */
        EXPECT_SUCCESS(s2n_negotiate_test_server_and_client(server_conn, client_conn));

        EXPECT_SUCCESS(s2n_shutdown_test_server_and_client(server_conn, client_conn));

        EXPECT_SUCCESS(s2n_config_free(client_config));
        EXPECT_SUCCESS(s2n_config_free(server_config));
        EXPECT_SUCCESS(s2n_connection_free(server_conn));
        EXPECT_SUCCESS(s2n_connection_free(client_conn));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_io_pair_close(&io_pair));
    }

    /* Self-Talk test: the client initiates a handshake with an X25519 share.
     * The server, however does not support x25519 and prefers P-256.
     * The server then sends a HelloRetryRequest that requires the
     * client to generate a key share on the P-256 curve. */
    if (s2n_is_evp_apis_supported()) {
        struct s2n_config *server_config;
        struct s2n_config *client_config;

        struct s2n_connection *server_conn;
        struct s2n_connection *client_conn;

        struct s2n_cert_chain_and_key *tls13_chain_and_key;
        char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
        char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

        /* Create nonblocking pipes */
        struct s2n_test_io_pair io_pair;
        EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));

        EXPECT_NOT_NULL(server_config = s2n_config_new());
        EXPECT_NOT_NULL(client_config = s2n_config_new());

        EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
        EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

        EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(client_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(server_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_disable_x509_verification(client_config));

        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(client_config, "20190801")); /* contains x25519 */
        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(server_config, "20190802")); /* does not contain x25519 */

        EXPECT_SUCCESS(s2n_connection_set_config(server_conn, server_config));
        EXPECT_SUCCESS(s2n_connection_set_config(client_conn, client_config));

        server_conn->x509_validator.skip_cert_validation = 1;
        client_conn->x509_validator.skip_cert_validation = 1;

        /* Force the HRR path */
        client_conn->security_policy_override = &security_policy_test_tls13_retry;

        EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

        /* Negotiate handshake */
        EXPECT_SUCCESS(s2n_negotiate_test_server_and_client(server_conn, client_conn));

        EXPECT_SUCCESS(s2n_shutdown_test_server_and_client(server_conn, client_conn));

        EXPECT_SUCCESS(s2n_config_free(client_config));
        EXPECT_SUCCESS(s2n_config_free(server_config));
        EXPECT_SUCCESS(s2n_connection_free(server_conn));
        EXPECT_SUCCESS(s2n_connection_free(client_conn));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_io_pair_close(&io_pair));
    }

    /* If a client receives a second HelloRetryRequest in the same connection
     * (i.e., where the ClientHello was itself in response to a HelloRetryRequest),
     * it MUST raise an error and abort the handshake. */
    {
        struct s2n_config *server_config;
        struct s2n_config *client_config;

        struct s2n_connection *server_conn;
        struct s2n_connection *client_conn;

        struct s2n_cert_chain_and_key *tls13_chain_and_key;
        char tls13_cert_chain[S2N_MAX_TEST_PEM_SIZE] = {0};
        char tls13_private_key[S2N_MAX_TEST_PEM_SIZE] = {0};

        EXPECT_NOT_NULL(server_config = s2n_config_new());
        EXPECT_NOT_NULL(client_config = s2n_config_new());

        EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
        EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

        EXPECT_NOT_NULL(tls13_chain_and_key = s2n_cert_chain_and_key_new());
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_CERT_CHAIN, tls13_cert_chain, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_read_test_pem(S2N_ECDSA_P384_PKCS1_KEY, tls13_private_key, S2N_MAX_TEST_PEM_SIZE));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_load_pem(tls13_chain_and_key, tls13_cert_chain, tls13_private_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(client_config, tls13_chain_and_key));
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(server_config, tls13_chain_and_key));

        EXPECT_SUCCESS(s2n_connection_set_config(server_conn, server_config));

        /* Force the HRR path */
        client_conn->security_policy_override = &security_policy_test_tls13_retry;

        /* ClientHello 1 */
        EXPECT_SUCCESS(s2n_client_hello_send(client_conn));
        EXPECT_SUCCESS(s2n_stuffer_copy(&client_conn->handshake.io, &server_conn->handshake.io,
                                        s2n_stuffer_data_available(&client_conn->handshake.io)));
        EXPECT_SUCCESS(s2n_client_hello_recv(server_conn));
        EXPECT_TRUE(s2n_is_hello_retry_handshake(server_conn));
        EXPECT_SUCCESS(s2n_set_connection_hello_retry_flags(server_conn));

        /* Server HelloRetryRequest 1 */
        EXPECT_SUCCESS(s2n_server_hello_retry_send(server_conn));
        EXPECT_SUCCESS(s2n_set_connection_hello_retry_flags(server_conn));

        EXPECT_SUCCESS(s2n_stuffer_wipe(&client_conn->handshake.io));
        EXPECT_SUCCESS(s2n_stuffer_copy(&server_conn->handshake.io, &client_conn->handshake.io,
                                        s2n_stuffer_data_available(&server_conn->handshake.io)));
        client_conn->handshake.message_number = HELLO_RETRY_MSG_NO;
        EXPECT_SUCCESS(s2n_server_hello_recv(client_conn));

        /* ClientHello 2 */
        EXPECT_SUCCESS(s2n_client_hello_send(client_conn));
        EXPECT_SUCCESS(s2n_stuffer_wipe(&server_conn->handshake.io));
        EXPECT_SUCCESS(s2n_stuffer_copy(&client_conn->handshake.io, &server_conn->handshake.io,
                                        s2n_stuffer_data_available(&client_conn->handshake.io)));
        EXPECT_SUCCESS(s2n_client_hello_recv(server_conn));

        /* Server HelloRetryRequest 2 */
        EXPECT_SUCCESS(s2n_server_hello_retry_send(server_conn));

        EXPECT_SUCCESS(s2n_stuffer_wipe(&client_conn->handshake.io));
        EXPECT_SUCCESS(s2n_stuffer_copy(&server_conn->handshake.io, &client_conn->handshake.io,
                                        s2n_stuffer_data_available(&server_conn->handshake.io)));

        EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_recv(client_conn), S2N_ERR_INVALID_HELLO_RETRY);

        EXPECT_SUCCESS(s2n_config_free(client_config));
        EXPECT_SUCCESS(s2n_config_free(server_config));
        EXPECT_SUCCESS(s2n_connection_free(server_conn));
        EXPECT_SUCCESS(s2n_connection_free(client_conn));
        EXPECT_SUCCESS(s2n_cert_chain_and_key_free(tls13_chain_and_key));
    }

    /* Test s2n_hello_retry_validate raises a S2N_ERR_INVALID_HELLO_RETRY error when
     * when conn->handshake_params.server_random is not set to the correct hello retry random value
     * specified in the RFC: https://tools.ietf.org/html/rfc8446#section-4.1.3 */
    {
        struct s2n_connection *conn;
        EXPECT_NOT_NULL(conn = s2n_connection_new(S2N_CLIENT));
        /* From RFC: https://tools.ietf.org/html/rfc8446#section-4.1.3 */
        const uint8_t not_hello_retry_request_random[S2N_TLS_RANDOM_DATA_LEN] = { 0 };
        EXPECT_MEMCPY_SUCCESS(conn->handshake_params.server_random, not_hello_retry_request_random,
                              S2N_TLS_RANDOM_DATA_LEN);

        EXPECT_FAILURE_WITH_ERRNO(s2n_hello_retry_validate(conn), S2N_ERR_INVALID_HELLO_RETRY);

        EXPECT_SUCCESS(s2n_connection_free(conn));
    }

    /* Test server_hello_receive raises a S2N_ERR_CIPHER_NOT_SUPPORTED error when the cipher suite supplied
     * by the Server Hello does not match the cipher suite supplied by the Hello Retry Request */
    {
        struct s2n_connection *server_conn;
        struct s2n_connection *client_conn;

        EXPECT_NOT_NULL(server_conn = s2n_connection_new(S2N_SERVER));
        EXPECT_NOT_NULL(client_conn = s2n_connection_new(S2N_CLIENT));

        /* A Hello Retry Request has been processed */
        EXPECT_SUCCESS(s2n_set_hello_retry_required(client_conn));
        client_conn->secure.cipher_suite = &s2n_tls13_aes_256_gcm_sha384;
        client_conn->server_protocol_version = S2N_TLS13;
        client_conn->handshake.handshake_type |= NEGOTIATED;
        client_conn->handshake.handshake_type |= FULL_HANDSHAKE;
        client_conn->handshake.message_number = SERVER_HELLO_MSG_NO;

        /* Server Hello with cipher suite that does not match Hello Retry Request cipher suite */
        server_conn->secure.cipher_suite = &s2n_tls13_chacha20_poly1305_sha256;
        EXPECT_SUCCESS(s2n_server_hello_send(server_conn));

        EXPECT_SUCCESS(s2n_stuffer_wipe(&client_conn->handshake.io));
        EXPECT_SUCCESS(s2n_stuffer_copy(&server_conn->handshake.io, &client_conn->handshake.io,
                                        s2n_stuffer_data_available(&server_conn->handshake.io)));

        EXPECT_FAILURE_WITH_ERRNO(s2n_server_hello_recv(client_conn), S2N_ERR_CIPHER_NOT_SUPPORTED);

        EXPECT_SUCCESS(s2n_connection_free(server_conn));
        EXPECT_SUCCESS(s2n_connection_free(client_conn));
    }

    /*
     * Self-Talk
     *
     *= https://tools.ietf.org/rfc/rfc8446#section-4.1.2
     *= type=test
     *# The client will also send a
     *# ClientHello when the server has responded to its ClientHello with a
     *# HelloRetryRequest.  In that case, the client MUST send the same
     *# ClientHello without modification
     */
    if (s2n_is_tls13_fully_supported()) {
        DEFER_CLEANUP(struct s2n_cert_chain_and_key *chain_and_key,
                s2n_cert_chain_and_key_ptr_free);
        EXPECT_SUCCESS(s2n_test_cert_chain_and_key_new(&chain_and_key,
                S2N_DEFAULT_ECDSA_TEST_CERT_CHAIN, S2N_DEFAULT_ECDSA_TEST_PRIVATE_KEY));

        DEFER_CLEANUP(struct s2n_config *config = s2n_config_new(),
                s2n_config_ptr_free);
        EXPECT_SUCCESS(s2n_config_add_cert_chain_and_key_to_store(config, chain_and_key));
        EXPECT_SUCCESS(s2n_config_set_cipher_preferences(config, "default_tls13"));
        EXPECT_SUCCESS(s2n_config_disable_x509_verification(config));

        /* Sanity Check: The server accepts an unchanged ClientHello */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Finish handshake */
            EXPECT_SUCCESS(s2n_negotiate_test_server_and_client(server_conn, client_conn));
        }

        /* Test: The server rejects a second ClientHello with changed message fields */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Change session id */
            client_conn->session_id[0]++;

            /* Expect failure because second client hello doesn't match */
            EXPECT_FAILURE_WITH_ERRNO(s2n_negotiate_test_server_and_client(server_conn, client_conn),
                    S2N_ERR_BAD_MESSAGE);
        }

        /* Test: The server rejects a second ClientHello with changed client random */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Change client random */
            client_conn->handshake_params.client_random[0]++;

            /* Expect failure because second client hello doesn't match */
            EXPECT_FAILURE_WITH_ERRNO(s2n_negotiate_test_server_and_client(server_conn, client_conn),
                    S2N_ERR_BAD_MESSAGE);
        }

        /* Test: The server rejects a second ClientHello with a changed extension */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));
            EXPECT_SUCCESS(s2n_set_server_name(client_conn, "localhost"));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Change server name */
            client_conn->server_name[0]++;

            /* Expect failure because second client hello doesn't match */
            EXPECT_FAILURE_WITH_ERRNO(s2n_negotiate_test_server_and_client(server_conn, client_conn),
                    S2N_ERR_BAD_MESSAGE);
        }

        /* Test: The server rejects a second ClientHello with a removed extension */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));
            EXPECT_SUCCESS(s2n_set_server_name(client_conn, "localhost"));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Clear server name.
             * Without a server name, we don't send the server name extension. */
            client_conn->server_name[0] = '\0';

            /* Expect failure because second client hello doesn't match */
            EXPECT_FAILURE_WITH_ERRNO(s2n_negotiate_test_server_and_client(server_conn, client_conn),
                    S2N_ERR_BAD_MESSAGE);
        }

        /* Test: The server rejects a second ClientHello with an added extension */
        {
            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, config));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            client_conn->security_policy_override = &security_policy_test_tls13_retry;

            /* Send ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));

            /* Add a server name.
             * Without a server name, we don't send the server name extension. */
            EXPECT_SUCCESS(s2n_set_server_name(client_conn, "localhost"));

            /* Expect failure because second client hello doesn't match */
            EXPECT_FAILURE_WITH_ERRNO(s2n_negotiate_test_server_and_client(server_conn, client_conn),
                    S2N_ERR_BAD_MESSAGE);
        }

        /* Test: If the initial ClientHello includes all extensions, so does the second ClientHello.
         *
         * This includes TLS1.2 extensions, since the ClientHello is sent before
         * the client knows what version the server will negotiate.
         *
         * We have to test with all extensions to ensure that an s2n server will never reject
         * an s2n client's second ClientHello.
         *
         * TLS1.2 and TLS1.3 session tickets are mutually exclusive and use different
         * extensions, so we test once with each.
         */
        for (size_t tls13_tickets = 0; tls13_tickets < 2; tls13_tickets++) {
            DEFER_CLEANUP(struct s2n_config *client_config = s2n_config_new(),
                    s2n_config_ptr_free);
            EXPECT_SUCCESS(s2n_config_disable_x509_verification(client_config));

            DEFER_CLEANUP(struct s2n_connection *server_conn = s2n_connection_new(S2N_SERVER),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(server_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(server_conn, config));

            DEFER_CLEANUP(struct s2n_connection *client_conn = s2n_connection_new(S2N_CLIENT),
                    s2n_connection_ptr_free);
            EXPECT_NOT_NULL(server_conn);
            EXPECT_SUCCESS(s2n_connection_set_blinding(client_conn, S2N_SELF_SERVICE_BLINDING));
            EXPECT_SUCCESS(s2n_connection_set_config(client_conn, client_config));

            struct s2n_test_io_pair io_pair = { 0 };
            EXPECT_SUCCESS(s2n_io_pair_init_non_blocking(&io_pair));
            EXPECT_SUCCESS(s2n_connections_set_io_pair(client_conn, server_conn, &io_pair));

            /* Force the HRR path */
            const struct s2n_security_policy security_policy_test_tls13_retry_with_pq = {
                .minimum_protocol_version = S2N_TLS11,
                .cipher_preferences = &cipher_preferences_pq_tls_1_1_2021_05_21,
                .kem_preferences = &kem_preferences_pq_tls_1_0_2021_05,
                .signature_preferences = &s2n_signature_preferences_20200207,
                .ecc_preferences = &ecc_preferences_for_retry,
            };
            client_conn->security_policy_override = &security_policy_test_tls13_retry_with_pq;

            /* Setup all extensions */
            uint8_t apn[] = "https";
            EXPECT_SUCCESS(s2n_config_set_cipher_preferences(client_config, "PQ-TLS-1-1-2021-05-21"));
            EXPECT_SUCCESS(s2n_config_set_status_request_type(client_config, S2N_STATUS_REQUEST_OCSP));
            EXPECT_SUCCESS(s2n_config_set_ct_support_level(client_config, S2N_CT_SUPPORT_REQUEST));
            EXPECT_SUCCESS(s2n_config_send_max_fragment_length(client_config, S2N_TLS_MAX_FRAG_LEN_4096));
            EXPECT_SUCCESS(s2n_config_set_session_tickets_onoff(client_config, 1));
            EXPECT_SUCCESS(s2n_set_server_name(client_conn, "localhost"));
            EXPECT_SUCCESS(s2n_connection_append_protocol_preference(client_conn, apn, sizeof(apn)));
            EXPECT_SUCCESS(s2n_connection_set_early_data_expected(client_conn));
            if (tls13_tickets) {
                EXPECT_OK(s2n_append_test_psk_with_early_data(client_conn, 1, &s2n_tls13_aes_256_gcm_sha384));
            }
            EXPECT_SUCCESS(s2n_connection_enable_quic(client_conn));
            /* Need to enable quic on both sides so they can communicate */
            EXPECT_SUCCESS(s2n_connection_enable_quic(server_conn));

            /* Send and receive ClientHello */
            s2n_blocked_status blocked = 0;
            EXPECT_OK(s2n_negotiate_until_message(client_conn, &blocked, SERVER_HELLO));
            EXPECT_OK(s2n_negotiate_until_message(server_conn, &blocked, HELLO_RETRY_MSG));

            /* All ClientHello extensions must be present (except very specific exceptions)  */
            s2n_extension_type_list *extensions = NULL;
            EXPECT_SUCCESS(s2n_extension_type_list_get(S2N_EXTENSION_LIST_CLIENT_HELLO, &extensions));
            for (size_t i = 0; i < extensions->count; i++) {
                uint16_t iana = extensions->extension_types[i]->iana_value;

                /* The cookie is a special case and only appears AFTER the retry */
                if (iana == TLS_EXTENSION_COOKIE) {
                    continue;
                }

                /* s2n-tls doesn't actually support sending this extension */
                if (iana == TLS_EXTENSION_RENEGOTIATION_INFO) {
                    EXPECT_EQUAL(s2n_client_renegotiation_info_extension.should_send,
                            &s2n_extension_never_send);
                    continue;
                }

                /* No pq extension if pq not enabled for the build */
                if (iana == TLS_EXTENSION_PQ_KEM_PARAMETERS && !s2n_pq_is_enabled()) {
                    continue;
                }

                /* TLS1.2 session tickets and TLS1.3 session tickets are mutually exclusive */
                if (tls13_tickets && iana == TLS_EXTENSION_SESSION_TICKET) {
                    continue;
                } else if (!tls13_tickets && (iana == TLS_EXTENSION_PRE_SHARED_KEY
                        || iana == TLS_EXTENSION_PSK_KEY_EXCHANGE_MODES
                        || iana == TLS_EXTENSION_EARLY_DATA)) {
                    continue;
                }

                bool extension_exists = false;
                EXPECT_SUCCESS(s2n_client_hello_has_extension(&server_conn->client_hello,
                        iana, &extension_exists));
                EXPECT_TRUE(extension_exists);
            }

            /* Expect successful retry */
            EXPECT_SUCCESS(s2n_negotiate_test_server_and_client(server_conn, client_conn));
        }
    }

    EXPECT_SUCCESS(s2n_disable_tls13_in_test());

    END_TEST();
}
