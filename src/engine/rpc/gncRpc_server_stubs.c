/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "gncRpc.h"

bool_t
gncrpc_version_1_svc(void *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_book_begin_1_svc(gncrpc_book_begin_args *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_book_load_1_svc(char *argp, gncrpc_book_load_ret *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_book_end_1_svc(char *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_account_begin_edit_1_svc(gncrpc_backend_guid *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_account_commit_edit_1_svc(gncrpc_commit_acct_args *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_txn_begin_edit_1_svc(gncrpc_backend_guid *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_txn_commit_edit_1_svc(gncrpc_commit_txn_args *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_txn_rollback_edit_1_svc(gncrpc_backend_guid *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_run_query_1_svc(gncrpc_query_args *argp, gncrpc_query_ret *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_sync1_1_svc(gncrpc_sync1_args *argp, gncrpc_sync1_ret *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_sync2_1_svc(gncrpc_sync2_args *argp, int *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
gncrpc_get_txns_1_svc(gncrpc_get_txns_args *argp, gncrpc_get_txns_ret *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

int
gncrpc_prog_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
