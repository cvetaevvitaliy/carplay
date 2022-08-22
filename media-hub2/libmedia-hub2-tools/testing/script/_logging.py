import logging

FORMAT	=	'[%(asctime)s][%(levelname)-8s]%(message)s'
logging.basicConfig( format = FORMAT, level = logging.DEBUG )

logger  =   logging.getLogger()

res_d	=	[
		{ 'func': logger.debug, 'result': '\033[1;32;40mOK\033[0m' },
		{ 'func': logger.warning, 'result': '\033[1;31;40mINVALID_PARAM\033[0m' },
		{ 'func': logger.warning, 'result': '\033[1;31;40mINVALID_SEQ\033[0m' },
		{ 'func': logger.warning, 'result': '\033[1;31;40mFUNC_NOT_IMPLEMENT\033[0m' },
		{ 'func': logger.warning, 'result': '\033[1;31;40mIPC_ERROR\033[0m' }
	]

core_d =   [
		{ 'func': logger.debug, 'event': '\033[1;32;40mMH_CORE_STARTED\033[0m' },
		{ 'func': logger.debug, 'event': '\033[1;32;40mMH_CORE_STOPED\033[0m' },
		{ 'func': logger.warning, 'event': '\033[1;31;40mMH_CORE_PLUGIN_NOT_FOUND\033[0m' },
		{ 'func': logger.warning, 'event': '\033[1;31;40mMH_CORE_PLUGIN_INVALID\033[0m' },
		{ 'func': logger.debug, 'event': '\033[1;32;40mMH_CORE_PLUGIN_LOAD_SUCCESS\033[0m' },
		{ 'func': logger.warning, 'event': '\033[1;31;40mMH_CORE_PLUGIN_LOAD_FAILED\033[0m }' }
    ]

dev_d  =   [
		{ 'func': logger.debug, 'event': '\033[1;32;40mMH_DEV_ATTACHED\033[0m' },
		{ 'func': logger.debug, 'event': '\033[1;31;40mMH_DEV_DETACHED\033[0m' }
    ]

#Output message
def output( message ):
    logger.debug( message )

#Output message and assert the result
def resultA( message, res ):
	res_d[ res ][ 'func' ](	message + ' ... [' + res_d[ res ][ 'result' ] + ']' )

def core( message, event ):
	core_d[ event ][ 'func' ]( message + '[' + core_d[ event ][ 'event' ] + ']' )

def attach( message, event ):
	dev_d[ event ][ 'func' ]( message + '[' + dev_d[ event ][ 'event' ] + ']' )

def detach( message ):
	dev_d[ 1 ][ 'func' ]( message + '[' + dev_d[ 1 ][ 'event' ] + ']' )
