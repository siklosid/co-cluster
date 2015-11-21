template <class ParamsT>
void app_init( ParamsT &params, int argc, char *argv[],
               const uint32_t prog_mask,
               const char *prg, const char *prg_group ) {
  app_init( prg, prg_group );
  // temporary log settings for possible error reporting while reading params
  set_log( INT_MAX, stderr, true, true);
  params.Fetch( argc, argv, prog_mask );
  set_log( params.log_level, params.log_destination, (params.use_colors != 0) );
}

