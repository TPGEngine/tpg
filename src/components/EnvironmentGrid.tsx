
import { useState } from 'react';
import { motion } from 'framer-motion';
import { useNavigate } from 'react-router-dom';
import EnvironmentCard from './EnvironmentCard';
import { environments } from '@/data/environments';

const EnvironmentGrid = () => {
  const navigate = useNavigate();
  const [selectedEnvironment, setSelectedEnvironment] = useState<number | null>(null);

  const handleSelectEnvironment = (id: number) => {
    setSelectedEnvironment(id);
    console.log(`Selected environment: ${id}`);
    // Redirect to the environment detail page
    navigate(`/environment/${id}`);
  };

  return (
    <section className="py-16 px-6 md:px-12 bg-gray-50/50">
      <div className="max-w-7xl mx-auto space-y-8">
        <motion.div 
          className="text-center space-y-2"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.5 }}
        >
          <span className="text-sm font-medium text-tpg-blue">
            Select an environment
          </span>
          <h2 className="text-2xl md:text-3xl font-semibold">
            MuJoCo Environments
          </h2>
        </motion.div>

        <motion.div 
          className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6 md:gap-8"
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ duration: 0.5, delay: 0.2 }}
        >
          {environments.map((env, index) => (
            <motion.div
              key={env.id}
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.5, delay: 0.1 * index }}
            >
              <EnvironmentCard
                name={env.name}
                image={env.image}
                onClick={() => handleSelectEnvironment(env.id)}
              />
            </motion.div>
          ))}
        </motion.div>
      </div>
    </section>
  );
};

export default EnvironmentGrid;
