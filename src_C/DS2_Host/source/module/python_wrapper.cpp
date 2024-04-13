#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/operators.h>

#include "../ds2/ds2_host_class.h"

namespace py = pybind11;
using namespace pybind11::literals;


PYBIND11_MODULE(ds2, handle) {

	handle.doc() = "Module name: DS2_H \n \
					Author:	Vladyslav Shapoval \n \
					Master Thesis 2024 ";

	py::register_exception<DS2Exception>(handle, "DS2Exception");
		//.def("err_code", [](const DS2Exception& self) {return self.err_code; });

	py::class_<ds2_host>(handle, "ds2_host")
		.def(py::init())
		.def("get_freq_coef", [](ds2_host& self) { return self.coef; })
		.def("check_commit", 
			[](ds2_host& self, const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(self.check_commit(r, ck, fi, wi, timestamp));
				rl.append(timestamp);
				return rl;
			},
			py::return_value_policy::take_ownership)
		.def("check_commit2",
			[](ds2_host& self, const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(self.check_commit2(r, ck, fi, wi, timestamp));
				rl.append(timestamp);
				return rl;
			},
			py::return_value_policy::take_ownership)
		.def("translate_exception", [](ds2_host& self) {return self.err_code; })
		.def("hash_msg", 
			[](ds2_host& self, std::string& msg) {
				return py::bytes(self.hash_msg(msg));
			},
			py::return_value_policy::take_ownership)
		.def("get_parties_num", &ds2_host::get_party_num)
		.def("set_pi_commit", &ds2_host::set_pi_commit)
		.def("set_ti_commit", &ds2_host::set_ti_commit)
		.def("set_fi_commit", &ds2_host::set_fi_commit)
		.def("set_pi_val", &ds2_host::set_pi_val)
		.def("set_ti_val", &ds2_host::set_ti_val)
		.def("set_ri_val", &ds2_host::set_ri_val)
		.def("set_zi_2_val", &ds2_host::set_zi_2_val)
		.def("set_zi_1_val", &ds2_host::set_zi_1_val)
		.def("set_msg",
			[](ds2_host& self, std::string& msg) {
				self.msg = msg;
			})
		.def("get_msg",
			[](ds2_host& self) {
				return py::bytes(self.msg);
			},
			py::return_value_policy::take_ownership)
		.def("get_rho",
			[](ds2_host& self) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(py::bytes(self.get_rho(timestamp)));
				rl.append(timestamp);
				return rl;
			}, 
			py::return_value_policy::take_ownership)
		.def("get_tr",
			[](ds2_host& self) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(py::bytes(self.get_tr(timestamp)));
				rl.append(timestamp);
				return rl;
			},
			py::return_value_policy::take_ownership)
		.def("get_c",
			[](ds2_host& self) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(py::bytes(self.get_c(timestamp)));
				rl.append(timestamp);
				return rl;
			},
			py::return_value_policy::take_ownership)
		.def("get_signature",
			[](ds2_host& self) {
				uint64_t timestamp = 0;
				py::list rl;
				rl.append(py::bytes(self.get_signature(timestamp)));
				rl.append(timestamp);
				return rl;
			},
			py::return_value_policy::take_ownership)
		.def("reset", &ds2_host::reset)
		.def("is_flag_ready", &ds2_host::is_flag_ready);
}