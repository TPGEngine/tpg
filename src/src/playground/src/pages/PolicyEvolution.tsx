
import { useEffect, useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { motion } from 'framer-motion';
import { ArrowLeft, Play, Pause, RefreshCw, List, Activity } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Separator } from '@/components/ui/separator';
import { getEnvironmentById, Environment } from '@/data/environments';
import { useToast } from '@/components/ui/use-toast';
import LogPanel from '@/components/LogPanel';
import VisualizationPanel from '@/components/VisualizationPanel';

const PolicyEvolution = () => {
  const { id } = useParams<{ id: string }>();
  const navigate = useNavigate();
  const { toast } = useToast();
  const [environment, setEnvironment] = useState<Environment | null>(null);
  const [loading, setLoading] = useState(true);
  const [isEvolving, setIsEvolving] = useState(false);
  const [generation, setGeneration] = useState(0);
  const [logs, setLogs] = useState<{ generation: number; fitness: number; timestamp: string }[]>([]);

  useEffect(() => {
    // Scroll to top when component mounts
    window.scrollTo(0, 0);
    
    if (id) {
      const envId = parseInt(id);
      const env = getEnvironmentById(envId);
      
      if (env) {
        setEnvironment(env);
      } else {
        // Environment not found, redirect to home
        navigate('/');
        toast({
          title: "Environment not found",
          description: "The requested environment could not be found.",
          variant: "destructive"
        });
      }
    }
    
    setLoading(false);
  }, [id, navigate, toast]);

  const handleToggleEvolution = () => {
    setIsEvolving(prev => !prev);
    toast({
      title: isEvolving ? "Evolution paused" : "Evolution started",
      description: isEvolving 
        ? `Paused evolution for ${environment?.name}` 
        : `Starting evolution for ${environment?.name}`,
    });
  };

  const handleReset = () => {
    setGeneration(0);
    setLogs([]);
    setIsEvolving(false);
    toast({
      title: "Evolution reset",
      description: `Reset evolution for ${environment?.name}`,
    });
  };

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="w-8 h-8 border-2 border-t-tpg-blue border-gray-200 rounded-full animate-spin"></div>
      </div>
    );
  }

  if (!environment) {
    return null;
  }

  return (
    <motion.div
      className="min-h-screen pt-24 pb-16 px-6 md:px-12"
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.4 }}
    >
      <div className="max-w-7xl mx-auto">
        {/* Back button */}
        <div className="mb-8">
          <Button 
            variant="ghost" 
            className="flex items-center gap-2 text-gray-600 hover:text-gray-900"
            onClick={() => navigate(`/environment/${id}`)}
          >
            <ArrowLeft className="h-4 w-4" />
            <span>Back to Environment</span>
          </Button>
        </div>
        
        {/* Page title */}
        <motion.h1 
          className="text-3xl md:text-4xl font-bold mb-8"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6 }}
        >
          Evolving Policy for MuJoCo {environment.name} Environment
        </motion.h1>

        {/* Control buttons */}
        <div className="flex flex-wrap gap-4 mb-8">
          <Button 
            variant={isEvolving ? "outline" : "default"}
            className="flex items-center gap-2"
            onClick={handleToggleEvolution}
          >
            {isEvolving ? <Pause className="h-4 w-4" /> : <Play className="h-4 w-4" />}
            <span>{isEvolving ? "Pause Evolution" : "Start Evolution"}</span>
          </Button>
          
          <Button 
            variant="outline" 
            className="flex items-center gap-2"
            onClick={handleReset}
          >
            <RefreshCw className="h-4 w-4" />
            <span>Reset</span>
          </Button>
        </div>
        
        {/* Generation counter */}
        <div className="mb-8">
          <div className="flex items-center gap-2">
            <Activity className="h-5 w-5 text-tpg-blue" />
            <span className="text-lg font-medium">Current Generation: {generation}</span>
          </div>
          {isEvolving && (
            <div className="mt-2 text-sm text-gray-500 flex items-center gap-2">
              <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse"></div>
              Evolution in progress...
            </div>
          )}
        </div>
        
        {/* Two-panel layout */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          {/* Logs panel */}
          <div className="bg-white rounded-xl p-6 shadow-sm border border-gray-100">
            <div className="flex items-center gap-2 mb-4">
              <List className="h-5 w-5 text-gray-700" />
              <h2 className="text-xl font-semibold">Evolution Logs</h2>
            </div>
            <Separator className="mb-4" />
            <LogPanel logs={logs} />
          </div>
          
          {/* Visualization panel */}
          <div className="bg-white rounded-xl p-6 shadow-sm border border-gray-100">
            <div className="flex items-center gap-2 mb-4">
              <Activity className="h-5 w-5 text-gray-700" />
              <h2 className="text-xl font-semibold">Agent Visualization</h2>
            </div>
            <Separator className="mb-4" />
            <VisualizationPanel 
              environmentName={environment.name}
              generation={generation}
              isActive={isEvolving}
            />
          </div>
        </div>
      </div>
    </motion.div>
  );
};

export default PolicyEvolution;
