/*
   namespace orwell{
   namespace tests{

   class Utils
   {
   SetupLogger(std::string & const iName)
   {
   log4cxx::PatternLayout aPatternLayout("%d %-5p (%F:%L) - %m%n");
   log4cxx::ConsoleAppender aConsoleAppender(&aPatternLayout);
   BasicConfigurator::configure(&aConsoleAppender);
   log4cxx::LoggerPtr  logger(log4cxx::Logger::getLogger("orwell.log.test." + iName));
   logger->setLevel(log4cxx::Level::getDebug());

   }









   }

   }}
   */
