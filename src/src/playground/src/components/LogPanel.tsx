
import { motion } from 'framer-motion';
import { ScrollArea } from "@/components/ui/scroll-area";

interface Log {
  generation: number;
  fitness: number;
  timestamp: string;
}

interface LogPanelProps {
  logs: Log[];
}

const LogPanel = ({ logs }: LogPanelProps) => {
  return (
    <ScrollArea className="h-[400px] rounded-md">
      {logs.length === 0 ? (
        <div className="h-full flex items-center justify-center text-gray-500 italic">
          No logs available yet. Start the evolution to generate logs.
        </div>
      ) : (
        <div className="space-y-3 pr-4">
          {logs.map((log, index) => (
            <motion.div
              key={`${log.generation}-${index}`}
              className="p-3 bg-gray-50 rounded-lg"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.3 }}
            >
              <div className="flex justify-between items-center mb-1">
                <span className="font-semibold text-gray-800">
                  Generation {log.generation}
                </span>
                <span className="text-xs text-gray-500">
                  {log.timestamp}
                </span>
              </div>
              <div className="text-sm text-gray-700">
                <span className="inline-flex items-center text-tpg-blue">
                  Fitness score: {log.fitness}
                </span>
              </div>
            </motion.div>
          ))}
        </div>
      )}
    </ScrollArea>
  );
};

export default LogPanel;
