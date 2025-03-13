
import { useEffect } from 'react';
import { motion } from 'framer-motion';
import Hero from '@/components/Hero';
import InfoLinks from '@/components/InfoLinks';
import EnvironmentGrid from '@/components/EnvironmentGrid';

const Index = () => {
  useEffect(() => {
    // Scroll to top when component mounts
    window.scrollTo(0, 0);
  }, []);

  return (
    <motion.div
      className="min-h-screen"
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.4 }}
    >
      <Hero />
      <InfoLinks />
      <EnvironmentGrid />
    </motion.div>
  );
};

export default Index;
