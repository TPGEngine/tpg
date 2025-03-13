
import { motion } from 'framer-motion';

interface VisualizationPanelProps {
  environmentName: string;
  generation: number;
  isActive: boolean;
}

const VisualizationPanel = ({ environmentName, generation, isActive }: VisualizationPanelProps) => {
  return (
    <div className="w-full h-[400px] bg-gray-50 rounded-lg overflow-hidden flex items-center justify-center">
      {!isActive ? (
        <motion.div 
          className="text-center p-6"
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ duration: 0.5 }}
        >
          <div className="text-gray-400 mb-4">
            Visualization Placeholder
          </div>
          <div className="text-sm text-gray-500">
            Start the evolution process to visualize the agent
          </div>
        </motion.div>
      ) : (
        <div className="w-full h-full flex flex-col items-center justify-center">
          <div className="text-lg font-medium mb-4">
            {environmentName} - Generation {generation}
          </div>
          <div className="text-gray-500">
            Agent visualization will be rendered here
          </div>
          <div className="mt-8 w-16 h-16 border-2 border-t-tpg-blue border-gray-200 rounded-full animate-spin"></div>
        </div>
      )}
    </div>
  );
};

export default VisualizationPanel;
