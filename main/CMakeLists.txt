idf_component_register(SRCS "sd_card_example_main.c" "ulp_wakeup.c"
                    INCLUDE_DIRS ".")
set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_s_sources "ulp/pulse_cnt.S" "ulp/wake_up.S")
set(ulp_exp_dep_srcs "sulp_wakeup.c")
ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")