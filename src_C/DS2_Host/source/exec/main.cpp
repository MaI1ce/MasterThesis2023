#include "../ds2/ds2_host_class.h"
#include "../ds2/ds2_node_class.h"

#include <cstdio>

int main()
{
	int res = 0;
	ds2_host* host = NULL;
	ds2_node* node1 = NULL;
	ds2_node* node2 = NULL;

	uint64_t timestamp = 0;

	try {
		host = new ds2_host();
		node1 = new ds2_node();
		node2 = new ds2_node();

		std::string msg = "test msg";

		std::string hash = host->hash_msg(msg);
		node1->set_msg(hash);
		node2->set_msg(hash);
		host->msg = hash;

		node1->node_id = 0;
		node2->node_id = 1;

		std::printf("-- START KEYGEN -- \n");

		host->set_pi_commit(0, node1->get_pi_commit());
		host->set_pi_commit(1, node2->get_pi_commit());

		if (host->is_flag_ready(DS2_Pi_COMMIT_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Pi_COMMIT, "DS2_ERROR_Pi_COMMIT 1");

		host->set_pi_val(0, node1->get_pi_val());
		host->set_pi_val(1, node2->get_pi_val());

		if (host->is_flag_ready(DS2_Pi_VALUE_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Pi_COMMIT, "DS2_ERROR_Pi_COMMIT 2");


		std::string rho = host->get_rho(timestamp);
		node1->set_rho(rho);
		node2->set_rho(rho);

		host->set_ti_commit(0, node1->get_ti_commit());
		host->set_ti_commit(1, node2->get_ti_commit());

		if (host->is_flag_ready(DS2_Ti_COMMIT_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Ti_COMMIT, "DS2_ERROR_Ti_COMMIT 1");

		host->set_ti_val(0, node1->get_ti_val());
		host->set_ti_val(1, node2->get_ti_val());

		if (host->is_flag_ready(DS2_Ti_VALUE_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Ti_COMMIT, "DS2_ERROR_Ti_COMMIT 2");

		std::string tr = host->get_tr(timestamp);
		node1->set_tr(tr);
		node2->set_tr(tr);

		std::printf("-- END KEYGEN -- \n");

		std::printf("-- START SIGN -- \n");

		host->set_fi_commit(0, node1->get_fi_commit());
		host->set_fi_commit(1, node2->get_fi_commit());

		if (host->is_flag_ready(DS2_Fi_COMMIT_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, "DS2_ERROR_Fi_COMMIT 1");

		std::string c = host->get_c(timestamp);
		node1->set_c(c);
		node2->set_c(c);

		host->set_zi_1_val(0, node1->get_zi_1_val());
		host->set_zi_1_val(1, node2->get_zi_1_val());

		if (host->is_flag_ready(DS2_Zi_1_VALUE_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, "DS2_Zi_1_VALUE_FLAG");


		host->set_zi_2_val(0, node1->get_zi_2_val());
		host->set_zi_2_val(1, node2->get_zi_2_val());

		if (host->is_flag_ready(DS2_Zi_2_VALUE_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, "DS2_Zi_2_VALUE_FLAG");

		host->set_ri_val(0, node1->get_ri_val());
		host->set_ri_val(1, node2->get_ri_val());

		if (host->is_flag_ready(DS2_Ri_VALUE_FLAG) == false)
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, "DS2_Ri_VALUE_FLAG");
		
		std::string signature = host->get_signature(timestamp);

		std::printf("-- END SIGN -- \n");

		std::printf("-- START VERIFY -- \n");

		std::vector<std::string> ri = { std::string((const char*)node1->r_seed, SEED_BYTES), std::string((const char*)node2->r_seed, SEED_BYTES) };
		std::string sign_c = std::string((const char*)host->c, sizeof(host->c));
		std::string sign_z1 = std::string((const char*)host->z1, sizeof(host->z1));
		std::string sign_z2 = std::string((const char*)host->z2, sizeof(host->z2));

		bool flag = host->verify(sign_c, sign_z1, sign_z2, ri, timestamp);

		if (flag) {
			std::printf("SIGNATURE IS OK\n");
		}
		else {
			std::printf("SIGNATURE IS NOK\n");
		}
		std::printf("-- END VERIFY -- \n");

		if (host != NULL)
			delete host;
		if (node1 != NULL)
			delete node1;
		if (node2 != NULL)
			delete node2;
	}
	catch (DS2Exception err) {
		std::printf(err.what());
		if (host != NULL)
			delete host;
		if (node1 != NULL)
			delete node1;
		if (node2 != NULL)
			delete node2;
		res = 1;
	}

	return res;
}