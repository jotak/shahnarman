cost = {0,1,0,0}
icon = "disenchant"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Désenchantement"
	else
		return "Disenchant"
	end
end

function getDescription()
	if language == "french" then
		return "Supprime l'enchantement ciblé."
	else
		return "Remove target enchantment."
	end
end

function onCast()
	selectTarget("spell_in_play", "")
end

-- this function receives playedId and spellId as parameter, in a space-separated string
function onResolve(params)
	player, spell = split(params, " ")
	discardActiveSpell(player, spell)
end
