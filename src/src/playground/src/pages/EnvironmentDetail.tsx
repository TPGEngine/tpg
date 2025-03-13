
import { useEffect, useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { motion } from 'framer-motion';
import { ArrowLeft, Play } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { getEnvironmentById, Environment } from '@/data/environments';
import { useToast } from '@/components/ui/use-toast';

const EnvironmentDetail = () => {
  const { id } = useParams<{ id: string }>();
  const navigate = useNavigate();
  const { toast } = useToast();
  const [environment, setEnvironment] = useState<Environment | null>(null);
  const [loading, setLoading] = useState(true);

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

  const handleEvolvePolicy = () => {
    toast({
      title: "Starting evolution",
      description: `Beginning policy evolution for ${environment?.name}...`,
    });
    // Navigate to the policy evolution page
    navigate(`/evolution/${id}`);
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
            onClick={() => navigate('/')}
          >
            <ArrowLeft className="h-4 w-4" />
            <span>Back to Environments</span>
          </Button>
        </div>
        
        {/* Environment title */}
        <motion.h1 
          className="text-3xl md:text-4xl font-bold mb-6"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6 }}
        >
          MuJoCo {environment.name}
        </motion.h1>
        
        {/* Environment preview */}
        <motion.div 
          className="aspect-video bg-gray-100 rounded-xl overflow-hidden mb-12 max-w-3xl"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, delay: 0.1 }}
        >
          <img
            src={environment.image}
            alt={environment.name}
            className="w-full h-full object-cover"
          />
        </motion.div>
        
        {/* Description section */}
        <motion.div
          className="mb-12 max-w-3xl"
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, delay: 0.2 }}
        >
          <h2 className="text-2xl font-semibold mb-4">Description</h2>
          <p className="text-gray-700 leading-relaxed">
            {environment.description}
          </p>
          
          <div className="mt-6">
            <span className="inline-flex items-center px-3 py-1 rounded-full text-sm font-medium bg-gray-100 text-gray-800">
              Complexity: {environment.complexity}
            </span>
          </div>
        </motion.div>
        
        {/* CTA button */}
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.6, delay: 0.3 }}
        >
          <Button 
            className="px-8 py-6 rounded-md text-lg bg-tpg-blue hover:bg-tpg-blue/90 flex items-center gap-2"
            onClick={handleEvolvePolicy}
          >
            <Play className="h-5 w-5" />
            <span>Evolve Policy</span>
          </Button>
        </motion.div>
      </div>
    </motion.div>
  );
};

export default EnvironmentDetail;
