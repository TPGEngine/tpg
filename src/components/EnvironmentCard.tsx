
import { useState } from 'react';
import { motion } from 'framer-motion';

interface EnvironmentCardProps {
  name: string;
  image: string;
  onClick: () => void;
}

const EnvironmentCard = ({ name, image, onClick }: EnvironmentCardProps) => {
  const [isLoading, setIsLoading] = useState(true);

  return (
    <motion.div
      className="environment-card cursor-pointer"
      onClick={onClick}
      whileHover={{ scale: 1.02 }}
      whileTap={{ scale: 0.98 }}
      initial={{ opacity: 0, scale: 0.9 }}
      animate={{ opacity: 1, scale: 1 }}
      transition={{ type: 'spring', stiffness: 300, damping: 20 }}
    >
      <div className="aspect-video relative overflow-hidden bg-gray-100 rounded-t-xl">
        {isLoading && (
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="w-8 h-8 border-2 border-gray-300 border-t-tpg-blue rounded-full animate-spin"></div>
          </div>
        )}
        <img
          src={image}
          alt={name}
          className={`w-full h-full object-cover transition-opacity duration-500 ${isLoading ? 'opacity-0' : 'opacity-100'}`}
          onLoad={() => setIsLoading(false)}
        />
        <div className="absolute inset-0 bg-gradient-to-t from-black/30 to-transparent opacity-0 hover:opacity-100 transition-opacity duration-300"></div>
      </div>
      <div className="p-4 text-center">
        <h3 className="font-medium text-base">{name}</h3>
      </div>
    </motion.div>
  );
};

export default EnvironmentCard;
