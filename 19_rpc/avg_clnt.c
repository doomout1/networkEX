/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "avg.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

double *
average_1(input_data *argp, CLIENT *clnt)
{
	static double clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, AVERAGE,
		(xdrproc_t) xdr_input_data, (caddr_t) argp,
		(xdrproc_t) xdr_double, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

void average_proc_1( char* host, int argc, char *argv[])
{
	CLIENT *clnt;
	double  *result_1, *dp, f;
	char *endptr;
	int i;
	input_data  data_list;
	data_list.input_data.input_data_val =
	 (double*) malloc(MAXAVGSIZE*sizeof(double));

	dp = data_list.input_data.input_data_val;
	data_list.input_data.input_data_len = argc - 2;
	for (i=1;i<=(argc - 2);i++)
	{
		f = strtod(argv[i+1],&endptr);
		printf("value	= %e\n",f);
		*dp = f;
		dp++;
	}
	clnt = clnt_create(host, AVERAGEPROG, AVERAGEVERS, "udp");
	if (clnt == NULL)
	{
	  clnt_pcreateerror(host);
	  exit(1);
	}
	result_1 = average_1(&data_list, clnt);
	if (result_1 == NULL)
	{
	  clnt_perror(clnt, "call failed:");
	}
	clnt_destroy( clnt );
	printf("average = %e\n",*result_1);
}

main( int argc, char* argv[] )
{
	char *host;

	if(argc < 3)
	{
	 printf(
	  "usage: %s server_host value ...\n",
	  argv[0]);
	  exit(1);
	}
	if(argc > MAXAVGSIZE + 2)
	{
	  printf("Two many input values\n");
	  exit(2);
	}
	host = argv[1];
	average_proc_1( host, argc, argv);
}
