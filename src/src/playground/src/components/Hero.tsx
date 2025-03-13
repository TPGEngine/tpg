
import { motion } from 'framer-motion';

const Hero = () => {
  return (
    <section className="pt-32 pb-16 px-6 md:px-12">
      <div className="max-w-4xl mx-auto text-center space-y-8">
        <motion.h1 
          className="text-4xl md:text-5xl lg:text-6xl font-bold"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6 }}
        >
          TPG Playground
        </motion.h1>
        
        <motion.p 
          className="text-lg text-gray-600 max-w-3xl mx-auto leading-relaxed"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, delay: 0.2 }}
        >
          An interactive platform for exploring MuJoCo environments and training
          them with the TPG Reinforcement Learning algorithm. Experiment with
          different environments and visualize the learning process.
        </motion.p>
      </div>
    </section>
  );
};

export default Hero;
