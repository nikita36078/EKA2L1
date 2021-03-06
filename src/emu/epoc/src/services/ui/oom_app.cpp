/*
 * Copyright (c) 2018 EKA2L1 Team
 * 
 * This file is part of EKA2L1 project
 * (see bentokun.github.com/EKA2L1).
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <common/cvt.h>
#include <common/log.h>

#include <epoc/services/ui/oom_app.h>
#include <epoc/services/window/window.h>

#include <common/e32inc.h>
#include <e32err.h>

#include <epoc/epoc.h>
#include <epoc/kernel.h>
#include <epoc/vfs.h>

namespace eka2l1 {
    oom_ui_app_server::oom_ui_app_server(eka2l1::system *sys)
        : service::server(sys, "101fdfae_10207218_AppServer", true) {
        REGISTER_IPC(oom_ui_app_server, get_layout_config_size, EAknEikAppUiLayoutConfigSize, "OOM::GetLayoutConfigSize");
        REGISTER_IPC(oom_ui_app_server, get_layout_config, EAknEikAppUiGetLayoutConfig, "OOM::GetLayoutConfig");
        REGISTER_IPC(oom_ui_app_server, set_sgc_params, EAknEikAppUiSetSgcParams, "OOM::SetSgcParams");
    }

    std::string oom_ui_app_server::get_layout_buf() {
        if (!winsrv) {
            winsrv = reinterpret_cast<window_server*>(&(*sys->get_kernel_system()->get_by_name<service::server>("!Windowserver")));
        }

        epoc::config::screen &scr_config = winsrv->get_current_focus_screen_config();

        akn_layout_config akn_config;

        akn_config.num_screen_mode = static_cast<int>(scr_config.modes.size());

        // TODO: Find out what this really does
        akn_config.num_hardware_mode = 0;

        // Static check on those pointer
        akn_config.screen_modes = sizeof(akn_layout_config);
        akn_config.hardware_infos = sizeof(akn_layout_config) + sizeof(akn_screen_mode_info) * akn_config.num_screen_mode;

        std::string result;
        result.append(reinterpret_cast<char *>(&akn_config), sizeof(akn_layout_config));

        for (std::size_t i = 0; i < scr_config.modes.size(); i++) {
            akn_screen_mode_info mode_info;

            // TODO: Change this based on user settings
            mode_info.loc = akn_softkey_loc::bottom;
            mode_info.mode_num = scr_config.modes[i].mode_number;
            mode_info.dmode = epoc::display_mode::color16ma;
            mode_info.info.orientation = epoc::number_to_orientation(scr_config.modes[i].rotation);
            mode_info.info.pixel_size = scr_config.modes[i].size;
            mode_info.info.twips_size = mode_info.info.pixel_size * twips_mul;

            result.append(reinterpret_cast<char *>(&mode_info), sizeof(akn_screen_mode_info));
        }

        // TODO: Hardware mode

        return result;
    }

    void oom_ui_app_server::get_layout_config_size(service::ipc_context &ctx) {
        layout_buf = get_layout_buf();

        int layout_buf_size = static_cast<int>(layout_buf.size());

        ctx.write_arg_pkg<int>(0, layout_buf_size);
        ctx.set_request_status(KErrNone);
    }

    void oom_ui_app_server::get_layout_config(service::ipc_context &ctx) {
        layout_buf = get_layout_buf();

        ctx.write_arg_pkg(0, reinterpret_cast<std::uint8_t *>(&layout_buf[0]),
            static_cast<std::uint32_t>(layout_buf.size()));

        ctx.set_request_status(KErrNone);
    }

    void oom_ui_app_server::set_sgc_params(service::ipc_context &ctx) {
        auto params_op = ctx.get_arg_packed<epoc::sgc_params>(0);

        if (!params_op) {
            ctx.set_request_status(KErrArgument);
            return;
        }

        params = std::move(*params_op);
        LOG_TRACE("OOM UI app server: set sgc params stubbed, returns KErrNone");

        ctx.set_request_status(KErrNone);
    }
}
