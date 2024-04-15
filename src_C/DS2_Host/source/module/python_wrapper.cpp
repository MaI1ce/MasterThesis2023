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
		.def("get_internal_state",
			[](ds2_host& self) {
				py::dict d(
					"c"_a = py::bytes(std::string((char*)self.c, sizeof(self.c))),
					"poly_c"_a = py::bytes(std::string((char*)&self.poly_c, sizeof(self.poly_c))),
					"ck_seed"_a = py::bytes(std::string((char*)self.ck_seed, sizeof(self.ck_seed))),
					"rho"_a = py::bytes(std::string((char*)self.rho, sizeof(self.rho))),
					"tr"_a = py::bytes(std::string((char*)self.tr, sizeof(self.tr))),
					"z1"_a = py::bytes(std::string((char*)self.z1, sizeof(self.z1))),
					"z2"_a = py::bytes(std::string((char*)self.z2, sizeof(self.z2))),
					"t1"_a = py::bytes(std::string((char*)self.t1, sizeof(self.t1))),
					"A"_a = py::bytes(std::string((char*)self.A, sizeof(self.A))),
					"r"_a = py::bytes(std::string((char*)self.r, sizeof(self.r))),
					"msg"_a = py::bytes(std::string((char*)self.msg.c_str(), sizeof(self.msg.size())))
				);

				return d;
			},
			py::return_value_policy::take_ownership)
		.def("set_public_key",
				[](ds2_host& self, const std::string& t1_, const std::string& rho_, const std::string& tr_) {
					self.set_public_key(t1_, rho_, tr_);
				})
		.def("get_public_key",
			[](ds2_host& self) {
				return py::bytes(self.get_publick_key());
			},
			py::return_value_policy::take_ownership)
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
			.def("verify",
				[](ds2_host& self, const std::string& c_, const std::string& z1_, const std::string& z2_, const std::vector<std::string> ri_) {
					uint64_t timestamp = 0;
					py::list rl;
					rl.append(self.verify(c_, z1_, z2_, ri_, timestamp));
					rl.append(timestamp);
					return rl;
				},
				py::return_value_policy::take_ownership)
		.def("reset", &ds2_host::reset)
		.def("is_flag_ready", &ds2_host::is_flag_ready);
}